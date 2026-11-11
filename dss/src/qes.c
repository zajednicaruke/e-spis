#include "qes.h"
#include "util.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int qes_analiziraj(const char *filepath, QES_Info *info) {
    if (!filepath || !info) return 0;
    memset(info, 0, sizeof(QES_Info));
    size_t len = 0;
    uint8_t *buf = ucitaj_datoteku(filepath, &len);
    if (!buf) return 0;
    int detektiran = 0;
    if (memmem_case(buf, len, "/Type/Sig", 9) || memmem_case(buf, len, "/ByteRange", 10)) {
        info->potpisan = 1;
        detektiran = 1;
        der_izvuci_certifikat(buf, len, &info->cert);
        akd_izvuci_meta(buf, len, &info->akd);
    }
    free(buf);
    return detektiran;
}

void qes_formatiraj(const QES_Info *info, char *out, size_t outlen) {
    if (!info || !out || outlen == 0) return;
    out[0] = '\0';
    if (!info->potpisan) return;
    if (info->cert.uspjesno && info->cert.cn[0]) {
        snprintf(out, outlen, "Potpisnik=%s | PIN=%s | OIB=%s", info->cert.cn, info->cert.serial, info->cert.oib);
    } else if (info->akd.uspjesno && info->akd.cn[0]) {
        snprintf(out, outlen, "Potpisnik=%s | Vrijeme=%s | CA=%s", info->akd.cn, info->akd.vrijeme, info->akd.o);
    } else {
        snprintf(out, outlen, "Potpisnik=QES Certifikat detektiran");
    }
}
