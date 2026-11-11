#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <openssl/sha.h>

void print_hex(const uint8_t *buf, size_t len) {
    for (size_t i = 0; i < len; i++) {
        printf("%02x", buf[i]);
    }
    printf("\n");
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Uporaba: %s <tbs.der>\n", argv[0]);
        return 1;
    }

    FILE *f = fopen(argv[1], "rb");
    if (!f) { perror("Greška otvaranja datoteke"); return 1; }
    fseek(f, 0, SEEK_END); long fsize = ftell(f); fseek(f, 0, SEEK_SET);
    uint8_t *buf = (uint8_t*)malloc(fsize);
    fread(buf, 1, fsize, f);
    fclose(f);

    uint8_t hash[SHA256_DIGEST_LENGTH];
    SHA256(buf, fsize, hash);

    printf("SHA-256 tbsCertificate hash: ");
    print_hex(hash, SHA256_DIGEST_LENGTH);

    free(buf);
    return 0;
}
