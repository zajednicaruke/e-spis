#include "qes.h"
#include "util.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int qes_analiziraj(const char *filepath, QES_Info *info) {
    memset(info, 0, sizeof(*info));

    FILE *f = fopen(filepath, "rb");
    if (!f) return 0;
    fseek(f, 0, SEEK_END);
    long vel = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (vel <= 0 || vel > 8 * 1024 * 1024) { fclose(f); return 0; }

    unsigned char *buf = malloc(vel);
    if (!buf) { fclose(f); return 0; }
    size_t procitano = fread(buf, 1, vel, f);
    fclose(f);
    if (procitano < 100) { free(buf); return 0; }

    #define IMA(s) (memmem_case(buf, procitano, s, sizeof(s)-1) != NULL)

    int ima_byterange = IMA("/ByteRange");
    int ima_pkcs7     = IMA("/adbe.pkcs7");
    int ima_cades     = IMA("/ETSI.CAdES");
    int ima_rfc3161   = IMA("/ETSI.RFC3161");
    int ima_type_sig  = IMA("/Type/Sig") || IMA("/Type /Sig");

    if (!ima_byterange && !ima_type_sig) {
        free(buf);
        return 0;
    }

    info->potpisan = 1;
    if (ima_cades)        strcpy(info->razina, "QES (CAdES)");
    else if (ima_rfc3161) strcpy(info->razina, "TSA (RFC3161)");
    else if (ima_pkcs7)   strcpy(info->razina, "PKCS7");
    else                  strcpy(info->razina, "NEPOZNATO");

    /* Kontekstualna pretraga — stara v3.0 logika */
    void *sig_marker = memmem_case(buf, procitano, "/SubFilter", 10);
    if (!sig_marker) sig_marker = memmem_case(buf, procitano, "/Type/Sig", 9);
    if (sig_marker) {
        const unsigned char *dstart = (const unsigned char*)sig_marker;
        size_t dmax = procitano - (dstart - buf);
        if (dmax > 4096) dmax = 4096;
        extract_pdf_string(dstart, dmax, "/SubFilter", info->subfilter, sizeof(info->subfilter));
        extract_pdf_string(dstart, dmax, "/Filter",    info->filter,    sizeof(info->filter));
        extract_pdf_string(dstart, dmax, "/Name",      info->potpisnik, sizeof(info->potpisnik));
        extract_pdf_string(dstart, dmax, "/M(",        info->vrijeme,   sizeof(info->vrijeme));

    /* GLOBALNI FALLBACK za /M — ako nije nadjen u kontekstu */
    if (info->vrijeme[0] == '\0') {
        void *pm = memmem_case(buf, procitano, "/M(", 3);
        if (pm) {
            unsigned char *qm = (unsigned char*)pm + 3;
            while (qm < buf + procitano && *qm != '(') qm++;
            if (qm < buf + procitano) qm++;
            size_t im = 0;
            while (qm < buf + procitano && im < sizeof(info->vrijeme) - 1 &&
                   *qm != ')' && *qm != '\n') {
                info->vrijeme[im++] = *qm++;
            }
            info->vrijeme[im] = '\0';
        }
    }
    }

    /* ByteRange */
    void *p = memmem_case(buf, procitano, "/ByteRange", 10);
    if (p) {
        unsigned char *q = (unsigned char*)p + 10;
        while (q < buf + procitano && *q != '[') q++;
        size_t i = 0;
        while (q < buf + procitano && i < sizeof(info->byte_range) - 1 &&
               *q != ']' && *q != '\n') {
            info->byte_range[i++] = *q++;
        }
        info->byte_range[i] = '\0';
    }

    /* NOVO: DER certifikat */
    der_izvuci_certifikat(buf, procitano, &info->cert);

    /* NOVO: AKD Certilia widget */
    akd_izvuci_meta(buf, procitano, &info->akd);

    /* Ako ime nije u /Name, uzmi ga iz AKD ili DER */
    if (info->potpisnik[0] == '\0' || strstr(info->potpisnik, "ANCHOR") ||
        strstr(info->potpisnik, "PAGE") || strstr(info->potpisnik, "Helv")) {
        if (info->akd.pronaden && info->akd.potpisnik[0]) {
            snprintf(info->potpisnik, sizeof(info->potpisnik),
                     "%s", info->akd.potpisnik);
        } else if (info->cert.uspjesno && info->cert.cn[0]) {
            snprintf(info->potpisnik, sizeof(info->potpisnik),
                     "%s", info->cert.cn);
        }
    }

    /* Fallback */
    if (info->potpisnik[0]  == '\0') strcpy(info->potpisnik,  "nepoznat");
    if (info->subfilter[0]  == '\0') strcpy(info->subfilter,  "nepoznat");
    if (info->filter[0]     == '\0') strcpy(info->filter,     "nepoznat");
    if (info->vrijeme[0]    == '\0') strcpy(info->vrijeme,    "nepoznato");
    if (info->byte_range[0] == '\0') strcpy(info->byte_range, "nepoznat");

    free(buf);
    return 1;
}

void qes_formatiraj(const QES_Info *info, char *out, size_t outlen) {
    if (!info->potpisan) {
        snprintf(out, outlen, "Nije potpisan");
        return;
    }

    /* Prioritet: AKD widget (čitljivo) > DER (kriptografski) > PDF /Name */
    char buf[1024];
    int n = 0;

    if (info->akd.pronaden && info->akd.potpisnik[0]) {
        n += snprintf(buf + n, sizeof(buf) - n, "Potpisnik=%s", info->akd.potpisnik);
    } else if (info->cert.uspjesno && info->cert.cn[0]) {
        n += snprintf(buf + n, sizeof(buf) - n, "Potpisnik=%s", info->cert.cn);
    }

    if (info->akd.pronaden && info->akd.potpisnik_pin[0]) {
        n += snprintf(buf + n, sizeof(buf) - n, "%sPIN=%s", n ? " | " : "",
                      info->akd.potpisnik_pin);
    }

    if (info->cert.uspjesno && info->cert.oib[0]) {
        n += snprintf(buf + n, sizeof(buf) - n, "%sOIB=%s", n ? " | " : "",
                      info->cert.oib);
    }

    if (info->akd.pronaden && info->akd.datum_potpisa[0]) {
        n += snprintf(buf + n, sizeof(buf) - n, "%sDatum=%s", n ? " | " : "",
                      info->akd.datum_potpisa);
    } else if (info->vrijeme[0]) {
        n += snprintf(buf + n, sizeof(buf) - n, "%sVrijeme=%s", n ? " | " : "",
                      info->vrijeme);
    }

    if (info->cert.uspjesno && info->cert.izdavatelj_cn[0]) {
        n += snprintf(buf + n, sizeof(buf) - n, "%sCA=%s", n ? " | " : "",
                      info->cert.izdavatelj_cn);
    } else if (info->akd.pronaden && info->akd.zaglavlje[0]) {
        n += snprintf(buf + n, sizeof(buf) - n, "%sAlat=%s", n ? " | " : "",
                      info->akd.zaglavlje);
    }

    if (info->cert.uspjesno && info->cert.not_before[0]) {
        n += snprintf(buf + n, sizeof(buf) - n, "%sValjan od %s", n ? " | " : "",
                      info->cert.not_before);
    }
    if (info->cert.uspjesno && info->cert.not_after[0]) {
        n += snprintf(buf + n, sizeof(buf) - n, " do %s", info->cert.not_after);
    }

    if (info->akd.pronaden && info->akd.provjera_url[0]) {
        n += snprintf(buf + n, sizeof(buf) - n, "%sProvjera=%s", n ? " | " : "",
                      info->akd.provjera_url);
    }

    snprintf(out, outlen, "%s", n ? buf : "Potpisan, ali bez metapodataka");
}
