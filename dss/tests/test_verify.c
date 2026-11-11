#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "verify.h"
#include "pdf.h"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Uporaba: %s <putanja_do_pdf>\n", argv[0]);
        return 1;
    }

    const char *pdf_filename = argv[1];
    FILE *f = fopen(pdf_filename, "rb");
    if (!f) {
        fprintf(stderr, "[XX] Greška: Ne mogu otvoriti datoteku %s\n", pdf_filename);
        return 1;
    }

    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);

    uint8_t *pdf_buf = (uint8_t*)malloc(fsize);
    if (!pdf_buf) {
        fclose(f);
        return 1;
    }

    size_t read_bytes = fread(pdf_buf, 1, fsize, f);
    fclose(f);

    char info_buf[1024] = {0};
    VerifyResult res = verify_pdf_signature(pdf_buf, read_bytes, info_buf, sizeof(info_buf));

    printf("==================================================\n");
    printf("  Zajednica RUKE — DSSvalidator Stvarna Verifikacija\n");
    printf("==================================================\n");
    printf("Dokument:   %s\n", pdf_filename);
    printf("Algoritam:  %s\n", verify_get_algorithm(pdf_buf, read_bytes));
    printf("Kod ishoda: %d\n", res);
    printf("--------------------------------------------------\n");
    printf("%s\n", info_buf);
    printf("==================================================\n\n");

    free(pdf_buf);
    return (res == VERIFY_SUCCESS) ? 0 : 1;
}
