#ifndef BIGNUM_H
#define BIGNUM_H
#include <stdint.h>
#include <stddef.h>

/* ============================================================
 * BigNum — veliki brojevi za RSA/ECDSA
 * Podržava do 4096 bita (128 x 32-bitnih riječi)
 * ============================================================ */

#define BN_MAX_WORDS 128

typedef struct {
    uint32_t w[BN_MAX_WORDS];
    size_t   len;
} BN;

void bn_init(BN *a);
void bn_zero(BN *a);
void bn_copy(BN *dst, const BN *src);
int  bn_is_zero(const BN *a);

void bn_from_bytes(BN *a, const uint8_t *bytes, size_t len);
void bn_to_bytes(const BN *a, uint8_t *out, size_t outlen);
void bn_to_hex(const BN *a, char *out, size_t outlen);

int  bn_cmp(const BN *a, const BN *b);
int  bn_bit(const BN *a, size_t bit);

void bn_add(BN *r, const BN *a, const BN *b);
void bn_sub(BN *r, const BN *a, const BN *b);
void bn_mul(BN *r, const BN *a, const BN *b);
void bn_mul_small(BN *r, const BN *a, uint32_t b);

void bn_mod(BN *r, const BN *a, const BN *m);
void bn_divmod(BN *q, BN *rem, const BN *a, const BN *b);
void bn_mod_add(BN *r, const BN *a, const BN *b, const BN *m);
void bn_mod_sub(BN *r, const BN *a, const BN *b, const BN *m);
void bn_mod_mul(BN *r, const BN *a, const BN *b, const BN *m);
void bn_mod_exp(BN *r, const BN *base, const BN *exp, const BN *mod);
void bn_mod_inv(BN *r, const BN *a, const BN *m);

#endif
