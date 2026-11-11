#include <stdio.h>
#include <string.h>
#include "bignum.h"

int main(void) {
    BN a, b, r;
    char hex[256];

    bn_init(&a); bn_init(&b); bn_init(&r);

    uint8_t a_bytes[] = {0x07, 0x5B, 0xCD, 0x15};
    bn_from_bytes(&a, a_bytes, sizeof(a_bytes));
    bn_to_hex(&a, hex, sizeof(hex));
    printf("a = %s (ocekivano 0x75bcd15)\n", hex);

    uint8_t b_bytes[] = {0x3A, 0xDE, 0x68, 0xB1};
    bn_from_bytes(&b, b_bytes, sizeof(b_bytes));
    bn_to_hex(&b, hex, sizeof(hex));
    printf("b = %s (ocekivano 0x3ade68b1)\n", hex);

    bn_mul(&r, &a, &b);
    bn_to_hex(&r, hex, sizeof(hex));
    printf("a*b = %s (ocekivano 0x1b140e8f78e6f95)\n", hex);

    BN thousand, seven, mod_result;
    bn_init(&thousand); bn_init(&seven); bn_init(&mod_result);
    bn_zero(&thousand); thousand.w[0] = 1000; thousand.len = 1;
    bn_zero(&seven); seven.w[0] = 7; seven.len = 1;
    bn_mod(&mod_result, &thousand, &seven);
    bn_to_hex(&mod_result, hex, sizeof(hex));
    printf("1000 mod 7 = %s (ocekivano 0x6)\n", hex);

    BN base, exp, mod, exp_result;
    bn_init(&base); bn_init(&exp); bn_init(&mod); bn_init(&exp_result);
    bn_zero(&base); base.w[0] = 2; base.len = 1;
    bn_zero(&exp); exp.w[0] = 10; exp.len = 1;
    bn_zero(&mod); mod.w[0] = 100000; mod.len = 1;
    bn_mod_exp(&exp_result, &base, &exp, &mod);
    bn_to_hex(&exp_result, hex, sizeof(hex));
    printf("2^10 mod 100000 = %s (ocekivano 0x400)\n", hex);

    return 0;
}
