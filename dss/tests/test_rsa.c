#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bignum.h"
#include "rsa.h"

/* Test: RSA-2048 public key parse + verify
   Generiraj s openssl:
   openssl genrsa -out key.pem 2048
   openssl rsa -in key.pem -pubout -outform DER -out pub.der
   echo "test" | openssl dgst -sha256 -sign key.pem > sig.bin
*/

int main(int argc, char **argv) {
    if (argc < 3) {
        printf("Upotreba: %s <pub.der> <sig.bin>\n", argv[0]);
        printf("Generiraj test:\n");
        printf("  openssl genrsa -out key.pem 2048\n");
        printf("  openssl rsa -in key.pem -pubout -outform DER -out pub.der\n");
        printf("  printf 'test' | openssl dgst -sha256 -sign key.pem > sig.bin\n");
        printf("  printf 'test' | openssl dgst -sha256 -binary > hash.bin\n");
        return 1;
    }

    /* Učitaj public key */
    FILE *f = fopen(argv[1], "rb");
    if (!f) { printf("Ne mogu otvoriti: %s\n", argv[1]); return 2; }
    fseek(f, 0, SEEK_END);
    long der_len = ftell(f);
    fseek(f, 0, SEEK_SET);
    uint8_t *der = malloc(der_len);
    fread(der, 1, der_len, f);
    fclose(f);

    RSA_PublicKey key;
    if (!rsa_parse_public_key(der, der_len, &key)) {
        printf("❌ Ne mogu parsirati public key\n");
        free(der);
        return 3;
    }
    char hex[512];
    bn_to_hex(&key.n, hex, sizeof(hex));
    printf("✅ Public key parsiran\n");
    printf("   n = %s\n", hex);
    bn_to_hex(&key.e, hex, sizeof(hex));
    printf("   e = %s\n", hex);
    free(der);

    /* Učitaj signature */
    f = fopen(argv[2], "rb");
    if (!f) { printf("Ne mogu otvoriti: %s\n", argv[2]); return 4; }
    fseek(f, 0, SEEK_END);
    long sig_len = ftell(f);
    fseek(f, 0, SEEK_SET);
    uint8_t *sig = malloc(sig_len);
    fread(sig, 1, sig_len, f);
    fclose(f);

    /* Hash "test" = SHA256("test") */
    /* 9F86D081884C7D659A2FEAA0C55AD015A3BF4F1B2B0B822CD15D6C15B0F00A08 */
    uint8_t hash[32] = {
        0x9f, 0x86, 0xd0, 0x81, 0x88, 0x4c, 0x7d, 0x65,
        0x9a, 0x2f, 0xea, 0xa0, 0xc5, 0x5a, 0xd0, 0x15,
        0xa3, 0xbf, 0x4f, 0x1b, 0x2b, 0x0b, 0x82, 0x2c,
        0xd1, 0x5d, 0x6c, 0x15, 0xb0, 0xf0, 0x0a, 0x08
    };

    int ok = rsa_verify_pkcs1_v15(&key, hash, 32, sig, sig_len);
    free(sig);

    if (ok) {
        printf("✅ RSA verify USPJEŠNO\n");
        return 0;
    } else {
        printf("❌ RSA verify NEUSPJEŠNO\n");
        return 5;
    }
}
