#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "pdf.h"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Uporaba: %s <putanja_do_pdf>\n", argv[0]);
        return 1;
    }

    FILE *f = fopen(argv[1], "rb");
    if (!f) { perror("Greška otvaranja PDF-a"); return 1; }
    fseek(f, 0, SEEK_END); long fsize = ftell(f); fseek(f, 0, SEEK_SET);
    uint8_t *buf = (uint8_t*)malloc(fsize);
    size_t r = fread(buf, 1, fsize, f);
    fclose(f);

    PdfSignatureData pdata;
    if (!pdf_extract_signature_data(buf, r, &pdata)) {
        fprintf(stderr, "[XX] Neuspjelo izvlačenje potpisa iz PDF-a.\n");
        free(buf); return 1;
    }

    // Spremamo cijeli izvađeni PKCS#7/CAdES binarni kontejner u datoteku
    FILE *out = fopen("pkcs7_container.bin", "wb");
    fwrite(pdata.sig_bytes, 1, pdata.sig_len, out);
    fclose(out);

    printf("[OK] Spremljen PKCS#7 kontejner (%zu bajtova).\n", pdata.sig_len);
    free(buf);
    return 0;
}
