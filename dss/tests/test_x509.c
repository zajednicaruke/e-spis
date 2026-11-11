#include <stdio.h>
#include <stdlib.h>
#include "x509_parser.h"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Uporaba: %s <cert_der.bin>\n", argv[0]);
        return 1;
    }

    FILE *f = fopen(argv[1], "rb");
    if (!f) { perror("Greška otvaranja certifikata"); return 1; }
    fseek(f, 0, SEEK_END); long fsize = ftell(f); fseek(f, 0, SEEK_SET);
    uint8_t *buf = (uint8_t*)malloc(fsize);
    fread(buf, 1, fsize, f);
    fclose(f);

    X509CertInfo cert;
    if (!x509_parse_cert(buf, fsize, &cert)) {
        fprintf(stderr, "[XX] Neuspjelo parsiranje X.509 certifikata.\n");
        free(buf); return 1;
    }

    // Ispis u formatu identičnom OpenSSL-u za provjeru diff-a
    printf("serial=%s\n", cert.serial_number);
    printf("notBefore=%s\n", cert.not_before);
    printf("notAfter=%s\n", cert.not_after);
    printf("subject=CN=%s, OIB=%s\n", cert.subject_name, cert.oib);

    free(buf);
    return 0;
}
