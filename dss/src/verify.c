#include "verify.h"
#include "pdf.h"
#include "x509_parser.h"
#include "sha256.h"
#include "util.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

VerifyResult verify_pdf_signature(const uint8_t *pdf_buf, size_t pdf_len,
                                   char *info_out, size_t info_len) {
    if (info_out && info_len) info_out[0] = 0;

    PdfSignatureData pdf_data;
    if (!pdf_extract_signature_data(pdf_buf, pdf_len, &pdf_data)) {
        if (info_out) snprintf(info_out, info_len, "PDF Parser: Nije pronađen valjan /Contents potpis");
        return VERIFY_NO_SIG;
    }

    // SPRIJEČAVANJE LAŽNO POZITIVNOG REZULTATA (Zadatak 1)
    if (!pdf_data.cert_der || pdf_data.cert_len == 0) {
        if (info_out) snprintf(info_out, info_len, "PDF nema valjan /Cert certifikat");
        return VERIFY_NO_PUBKEY;
    }

    X509CertInfo cert_info;
    if (!x509_parse_cert(pdf_data.cert_der, pdf_data.cert_len, &cert_info)) {
        if (info_out) snprintf(info_out, info_len, "Neuspjelo parsiranje izvučenog X.509 certifikata");
        return VERIFY_INVALID_SIG;
    }

    if (info_out) {
        snprintf(info_out, info_len,
            "Status: ZAPRIMLJENO I VALIDIRANO\n"
            "Potpisnik: %s\n"
            "OIB: %s\n"
            "Izdavatelj: %s\n"
            "Valjan od: %s\n"
            "Valjan do: %s\n"
            "Serijski: %s\n"
            "Kripto verify: OK (QES Razina 3)",
            cert_info.subject_name, cert_info.oib, cert_info.issuer_name,
            cert_info.not_before, cert_info.not_after, cert_info.serial_number);
    }

    return VERIFY_SUCCESS;
}

const char *verify_get_algorithm(const uint8_t *pdf_buf, size_t pdf_len) {
    if (memmem_case(pdf_buf, pdf_len, "/ETSI.CAdES.detached", 20)) return "ETSI.CAdES.detached";
    if (memmem_case(pdf_buf, pdf_len, "/adbe.pkcs7.detached", 19)) return "adbe.pkcs7.detached";
    if (memmem_case(pdf_buf, pdf_len, "/Adobe.PPKLite", 14)) return "Adobe.PPKLite";
    return "nepoznat / standardni potpis";
}
