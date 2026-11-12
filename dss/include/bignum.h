#ifndef BIGNUM_H
#define BIGNUM_H

#include <stdint.h>
#include <stddef.h>

#define BN_MAX_WORDS 16

typedef struct {
    uint32_t w[BN_MAX_WORDS];
    size_t   len;
} BN;

void bn_zero(BN *a);
void bn_copy(BN *dst, const BN *src);
int  bn_is_zero(const BN *a);
void bn_clamp(BN *a);
int  bn_cmp(const BN *a, const BN *b);

void bn_add(BN *r, const BN *a, const BN *b);
void bn_sub(BN *r, const BN *a, const BN *b);
void bn_mul(BN *r, const BN *a, const BN *b);
void bn_divmod(BN *q, BN *rem, const BN *a, const BN *b);
void bn_mod(BN *r, const BN *a, const BN *m);

void bn_mod_add(BN *r, const BN *a, const BN *b, const BN *m);
void bn_mod_sub(BN *r, const BN *a, const BN *b, const BN *m);
void bn_mod_mul(BN *r, const BN *a, const BN *b, const BN *m);
void bn_mod_inv(BN *r, const BN *a, const BN *m);

#endif
