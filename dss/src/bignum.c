#include "bignum.h"
#include <string.h>

void bn_zero(BN *a) {
    if (!a) return;
    memset(a->w, 0, sizeof(a->w));
    a->len = 1;
}

void bn_copy(BN *dst, const BN *src) {
    if (!dst || !src) return;
    memcpy(dst->w, src->w, sizeof(dst->w));
    dst->len = src->len;
}

int bn_is_zero(const BN *a) {
    if (!a) return 1;
    for (size_t i = 0; i < a->len; i++) {
        if (a->w[i] != 0) return 0;
    }
    return 1;
}

void bn_clamp(BN *a) {
    if (!a) return;
    while (a->len > 1 && a->w[a->len - 1] == 0) {
        a->len--;
    }
}

int bn_cmp(const BN *a, const BN *b) {
    if (!a || !b) return 0;
    size_t l1 = a->len, l2 = b->len;
    while (l1 > 1 && a->w[l1 - 1] == 0) l1--;
    while (l2 > 1 && b->w[l2 - 1] == 0) l2--;
    if (l1 > l2) return 1;
    if (l1 < l2) return -1;
    for (size_t i = l1; i > 0; i--) {
        if (a->w[i - 1] > b->w[i - 1]) return 1;
        if (a->w[i - 1] < b->w[i - 1]) return -1;
    }
    return 0;
}

void bn_add(BN *r, const BN *a, const BN *b) {
    BN res;
    bn_zero(&res);
    size_t max_len = (a->len > b->len) ? a->len : b->len;
    uint64_t carry = 0;
    for (size_t i = 0; i < max_len || carry; i++) {
        uint64_t sum = carry;
        if (i < a->len) sum += a->w[i];
        if (i < b->len) sum += b->w[i];
        res.w[i] = (uint32_t)(sum & 0xFFFFFFFF);
        carry = sum >> 32;
        res.len = i + 1;
    }
    bn_clamp(&res);
    bn_copy(r, &res);
}

void bn_sub(BN *r, const BN *a, const BN *b) {
    if (bn_cmp(a, b) < 0) {
        bn_zero(r);
        return;
    }
    BN res;
    bn_zero(&res);
    int64_t borrow = 0;
    for (size_t i = 0; i < a->len; i++) {
        int64_t diff = (int64_t)a->w[i] - borrow;
        if (i < b->len) diff -= b->w[i];
        if (diff < 0) {
            diff += 0x100000000LL;
            borrow = 1;
        } else {
            borrow = 0;
        }
        res.w[i] = (uint32_t)(diff & 0xFFFFFFFF);
        res.len = i + 1;
    }
    bn_clamp(&res);
    bn_copy(r, &res);
}

void bn_mul(BN *r, const BN *a, const BN *b) {
    BN res;
    bn_zero(&res);
    if (bn_is_zero(a) || bn_is_zero(b)) {
        bn_copy(r, &res);
        return;
    }
    for (size_t i = 0; i < a->len; i++) {
        uint64_t carry = 0;
        for (size_t j = 0; j < b->len || carry; j++) {
            uint64_t cur = res.w[i + j] + carry;
            if (j < b->len) cur += (uint64_t)a->w[i] * b->w[j];
            res.w[i + j] = (uint32_t)(cur & 0xFFFFFFFF);
            carry = cur >> 32;
            if (i + j + 1 > res.len) res.len = i + j + 1;
        }
    }
    bn_clamp(&res);
    bn_copy(r, &res);
}

void bn_divmod(BN *q, BN *rem, const BN *a, const BN *b) {
    BN Q, R;
    bn_zero(&Q);
    bn_zero(&R);
    if (bn_is_zero(b)) {
        if (q) bn_zero(q);
        if (rem) bn_zero(rem);
        return;
    }
    for (size_t i = a->len * 32; i > 0; i--) {
        size_t bit_idx = i - 1;
        uint32_t bit = (a->w[bit_idx / 32] >> (bit_idx % 32)) & 1;
        
        /* R = R << 1 | bit */
        uint64_t carry = bit;
        for (size_t j = 0; j < R.len || carry; j++) {
            uint64_t val = ((uint64_t)R.w[j] << 1) | carry;
            R.w[j] = (uint32_t)(val & 0xFFFFFFFF);
            carry = val >> 32;
            if (j + 1 > R.len) R.len = j + 1;
        }
        bn_clamp(&R);

        if (bn_cmp(&R, b) >= 0) {
            bn_sub(&R, &R, b);
            Q.w[bit_idx / 32] |= (1U << (bit_idx % 32));
            if ((bit_idx / 32) + 1 > Q.len) Q.len = (bit_idx / 32) + 1;
        }
    }
    bn_clamp(&Q);
    bn_clamp(&R);
    if (q) bn_copy(q, &Q);
    if (rem) bn_copy(rem, &R);
}

void bn_mod(BN *r, const BN *a, const BN *m) {
    bn_divmod(NULL, r, a, m);
}

void bn_mod_add(BN *r, const BN *a, const BN *b, const BN *m) {
    BN sum;
    bn_add(&sum, a, b);
    bn_mod(r, &sum, m);
}

void bn_mod_sub(BN *r, const BN *a, const BN *b, const BN *m) {
    BN a_mod, b_mod;
    bn_mod(&a_mod, a, m);
    bn_mod(&b_mod, b, m);
    if (bn_cmp(&a_mod, &b_mod) >= 0) {
        BN diff;
        bn_sub(&diff, &a_mod, &b_mod);
        bn_copy(r, &diff);
    } else {
        BN diff, tmp;
        bn_sub(&diff, &b_mod, &a_mod);
        bn_sub(&tmp, m, &diff);
        bn_copy(r, &tmp);
    }
}

void bn_mod_mul(BN *r, const BN *a, const BN *b, const BN *m) {
    BN prod;
    bn_mul(&prod, a, b);
    bn_mod(r, &prod, m);
}

/* Stabilni EEA modulni inverz: R = A^-1 mod M */
void bn_mod_inv(BN *r, const BN *a, const BN *m) {
    BN t, newt, r_, newr, q, tmp, prod;

    bn_zero(&t);
    bn_zero(&newt); newt.w[0] = 1; newt.len = 1;

    bn_copy(&r_, m);
    bn_mod(&newr, a, m);

    while (!bn_is_zero(&newr)) {
        bn_divmod(&q, &tmp, &r_, &newr);

        bn_copy(&r_, &newr);
        bn_copy(&newr, &tmp);

        bn_mod_mul(&prod, &q, &newt, m);
        bn_mod_sub(&tmp, &t, &prod, m);

        bn_copy(&t, &newt);
        bn_copy(&newt, &tmp);
    }

    bn_copy(r, &t);
}
