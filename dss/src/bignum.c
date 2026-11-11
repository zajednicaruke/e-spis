#include "bignum.h"
#include <string.h>
#include <stdio.h>

void bn_init(BN *a) { bn_zero(a); }

void bn_zero(BN *a) {
    memset(a->w, 0, sizeof(a->w));
    a->len = 1;
}

void bn_copy(BN *dst, const BN *src) {
    if (dst == src) return;
    memcpy(dst->w, src->w, sizeof(dst->w));
    dst->len = src->len;
}

int bn_is_zero(const BN *a) {
    for (size_t i = 0; i < a->len; i++)
        if (a->w[i] != 0) return 0;
    return 1;
}

static void bn_normalize(BN *a) {
    while (a->len > 1 && a->w[a->len - 1] == 0) a->len--;
}

void bn_from_bytes(BN *a, const uint8_t *bytes, size_t len) {
    bn_zero(a);
    if (len == 0) return;
    size_t words = (len + 3) / 4;
    if (words > BN_MAX_WORDS) words = BN_MAX_WORDS;
    a->len = words;
    for (size_t i = 0; i < len; i++) {
        size_t bit = (len - 1 - i) * 8;
        size_t word = bit / 32;
        size_t shift = bit % 32;
        if (word < BN_MAX_WORDS)
            a->w[word] |= (uint32_t)bytes[i] << shift;
    }
    bn_normalize(a);
}

void bn_to_bytes(const BN *a, uint8_t *out, size_t outlen) {
    memset(out, 0, outlen);
    for (size_t i = 0; i < a->len; i++) {
        size_t byte_idx = outlen - 1 - i * 4;
        if (i * 4 + 0 < outlen) out[byte_idx - 0] = (a->w[i]      ) & 0xFF;
        if (i * 4 + 1 < outlen) out[byte_idx - 1] = (a->w[i] >>  8) & 0xFF;
        if (i * 4 + 2 < outlen) out[byte_idx - 2] = (a->w[i] >> 16) & 0xFF;
        if (i * 4 + 3 < outlen) out[byte_idx - 3] = (a->w[i] >> 24) & 0xFF;
    }
}

void bn_to_hex(const BN *a, char *out, size_t outlen) {
    if (outlen < 3) { if (outlen) out[0] = 0; return; }
    out[0] = '0'; out[1] = 'x';
    size_t pos = 2;
    int started = 0;
    for (int i = (int)a->len - 1; i >= 0; i--) {
        for (int shift = 28; shift >= 0; shift -= 4) {
            int digit = (a->w[i] >> shift) & 0xF;
            if (digit == 0 && !started && i != 0) continue;
            started = 1;
            if (pos + 1 < outlen) {
                out[pos++] = digit < 10 ? '0' + digit : 'a' + digit - 10;
            }
        }
    }
    if (pos < outlen) out[pos] = 0;
}

int bn_cmp(const BN *a, const BN *b) {
    if (a->len != b->len) return a->len < b->len ? -1 : 1;
    for (int i = (int)a->len - 1; i >= 0; i--) {
        if (a->w[i] != b->w[i]) return a->w[i] < b->w[i] ? -1 : 1;
    }
    return 0;
}

int bn_bit(const BN *a, size_t bit) {
    size_t word = bit / 32;
    size_t shift = bit % 32;
    if (word >= a->len) return 0;
    return (a->w[word] >> shift) & 1;
}

void bn_add(BN *r, const BN *a, const BN *b) {
    uint64_t carry = 0;
    size_t max = a->len > b->len ? a->len : b->len;
    r->len = max;
    for (size_t i = 0; i < max; i++) {
        uint64_t sum = carry;
        if (i < a->len) sum += a->w[i];
        if (i < b->len) sum += b->w[i];
        r->w[i] = (uint32_t)sum;
        carry = sum >> 32;
    }
    if (carry && r->len < BN_MAX_WORDS) {
        r->w[r->len++] = (uint32_t)carry;
    }
    bn_normalize(r);
}

void bn_sub(BN *r, const BN *a, const BN *b) {
    int64_t borrow = 0;
    r->len = a->len;
    for (size_t i = 0; i < a->len; i++) {
        int64_t diff = (int64_t)a->w[i] - borrow;
        if (i < b->len) diff -= b->w[i];
        if (diff < 0) {
            diff += ((int64_t)1 << 32);
            borrow = 1;
        } else {
            borrow = 0;
        }
        r->w[i] = (uint32_t)diff;
    }
    bn_normalize(r);
}

void bn_mul(BN *r, const BN *a, const BN *b) {
    BN tmp;
    bn_zero(&tmp);
    tmp.len = a->len + b->len;
    if (tmp.len > BN_MAX_WORDS) tmp.len = BN_MAX_WORDS;
    for (size_t i = 0; i < a->len; i++) {
        uint64_t carry = 0;
        for (size_t j = 0; j < b->len && i + j < BN_MAX_WORDS; j++) {
            uint64_t prod = (uint64_t)a->w[i] * b->w[j] + tmp.w[i+j] + carry;
            tmp.w[i+j] = (uint32_t)prod;
            carry = prod >> 32;
        }
        size_t k = i + b->len;
        while (carry != 0 && k < BN_MAX_WORDS) {
            uint64_t sum = (uint64_t)tmp.w[k] + carry;
            tmp.w[k] = (uint32_t)sum;
            carry = sum >> 32;
            k++;
        }
    }
    bn_normalize(&tmp);
    bn_copy(r, &tmp);
}

void bn_mul_small(BN *r, const BN *a, uint32_t b) {
    uint64_t carry = 0;
    r->len = a->len;
    for (size_t i = 0; i < a->len; i++) {
        uint64_t prod = (uint64_t)a->w[i] * b + carry;
        r->w[i] = (uint32_t)prod;
        carry = prod >> 32;
    }
    if (carry && r->len < BN_MAX_WORDS) r->w[r->len++] = (uint32_t)carry;
    bn_normalize(r);
}

/* R = A mod M — binary long division */
void bn_mod_add(BN *r, const BN *a, const BN *b, const BN *m) {
    BN sum;
    bn_add(&sum, a, b);
    bn_mod(r, &sum, m);
}

void bn_mod_sub(BN *r, const BN *a, const BN *b, const BN *m) {
    if (bn_cmp(a, b) >= 0) {
        BN diff;
        bn_sub(&diff, a, b);
        bn_mod(r, &diff, m);
    } else {
        BN tmp;
        bn_copy(&tmp, m);
        bn_add(&tmp, &tmp, a);
        bn_sub(&tmp, &tmp, b);
        bn_mod(r, &tmp, m);
    }
}

void bn_mod_mul(BN *r, const BN *a, const BN *b, const BN *m) {
    BN prod;
    bn_mul(&prod, a, b);
    bn_mod(r, &prod, m);
}

/* R = base^exp mod m — square-and-multiply */
void bn_mod_exp(BN *r, const BN *base, const BN *exp, const BN *mod) {
    BN result;
    bn_zero(&result);
    result.w[0] = 1;
    result.len = 1;

    BN b;
    bn_mod(&b, base, mod);

    /* Nađi najviši bit exp */
    int top_bit = (int)(exp->len * 32);
    while (top_bit > 0 && !bn_bit(exp, top_bit - 1)) top_bit--;

    for (int i = top_bit - 1; i >= 0; i--) {
        bn_mod_mul(&result, &result, &result, mod);
        if (bn_bit(exp, i)) {
            bn_mod_mul(&result, &result, &b, mod);
        }
    }
    bn_copy(r, &result);
}

/* R = a^-1 mod m — Fermat (m prime) ili extended GCD */

/* ============================================================
 * bn_divmod — binary long division, vraća kvocijent i ostatak
 * ============================================================ */
void bn_divmod(BN *q, BN *rem, const BN *a, const BN *b) {
    if (bn_is_zero(b)) { bn_zero(q); bn_zero(rem); return; }
    if (bn_cmp(a, b) < 0) { bn_zero(q); bn_copy(rem, a); return; }

    int top_bit = (int)(a->len * 32);
    while (top_bit > 0 && !bn_bit(a, top_bit - 1)) top_bit--;

    BN qq, rr;
    bn_zero(&qq);
    bn_zero(&rr);

    for (int i = top_bit - 1; i >= 0; i--) {
        /* rr <<= 1 */
        uint32_t carry = 0;
        for (size_t j = 0; j < rr.len; j++) {
            uint32_t nc = rr.w[j] >> 31;
            rr.w[j] = (rr.w[j] << 1) | carry;
            carry = nc;
        }
        if (carry && rr.len < BN_MAX_WORDS) rr.w[rr.len++] = carry;

        /* rr |= bit i od a */
        if (bn_bit(a, i)) {
            if (rr.len == 0) { rr.w[0] = 1; rr.len = 1; }
            else rr.w[0] |= 1;
        }

        /* if rr >= b: rr -= b, qq bit i = 1 */
        if (bn_cmp(&rr, b) >= 0) {
            bn_sub(&rr, &rr, b);
            size_t word = i / 32, bit = i % 32;
            while (qq.len <= word) qq.w[qq.len++] = 0;
            qq.w[word] |= (1u << bit);
        }
    }

    bn_normalize(&qq);
    bn_normalize(&rr);
    bn_copy(q, &qq);
    bn_copy(rem, &rr);
}

/* ============================================================
 * bn_mod — R = A mod M (koristi bn_divmod)
 * ============================================================ */
void bn_mod(BN *r, const BN *a, const BN *m) {
    BN q;
    bn_divmod(&q, r, a, m);
}

/* ============================================================
 * bn_mod_inv — R = A^-1 mod M (extended GCD, radi mod m)
 * ============================================================ */
void bn_mod_inv(BN *r, const BN *a, const BN *m) {
    BN t, newt, r_, newr, q, tmp, prod;

    bn_zero(&t);
    bn_zero(&newt); newt.w[0] = 1; newt.len = 1;

    bn_copy(&r_, m);
    bn_mod(&newr, a, m);

    while (!bn_is_zero(&newr)) {
        /* q = r_ / newr, tmp = r_ mod newr */
        bn_divmod(&q, &tmp, &r_, &newr);

        /* (r_, newr) = (newr, tmp) */
        bn_copy(&r_, &newr);
        bn_copy(&newr, &tmp);

        /* (t, newt) = (newt, (t - q*newt) mod m) */
        bn_mul(&prod, &q, &newt);
        /* tmp = (t - q*newt) mod m */
        /* Kako je t < m, a prod može biti velik: (t + m*k - prod) mod m */
        /* Sigurno: (t mod m) - (prod mod m), pa mod m */
        BN t_mod, prod_mod;
        bn_mod(&t_mod, &t, m);
        bn_mod(&prod_mod, &prod, m);
        if (bn_cmp(&t_mod, &prod_mod) >= 0) {
            bn_sub(&tmp, &t_mod, &prod_mod);
        } else {
            bn_add(&tmp, &t_mod, m);
            bn_sub(&tmp, &tmp, &prod_mod);
        }
        bn_copy(&t, &newt);
        bn_copy(&newt, &tmp);
    }

    bn_mod(r, &t, m);
}

