#include "ecdsa_p256.h"
#include <string.h>
#include <stdio.h>

/* NIST P-256 konstante */
static const uint32_t P256_P_W[8] = {
    0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0x00000000,
    0x00000000, 0x00000000, 0x00000001, 0xFFFFFFFF
};

static const uint32_t P256_N_W[8] = {
    0xFC632551, 0xF3B9CAC2, 0xA7179E84, 0xBCE6FAAD,
    0xFFFFFFFF, 0xFFFFFFFF, 0x00000000, 0xFFFFFFFF
};

/* Bazna točka G (Affine) */
static const uint32_t P256_GX_W[8] = {
    0xD898C296, 0xF4A13945, 0x2DEB33A0, 0x77037D81,
    0x63A440F2, 0xF8BCE6E5, 0xE12C4247, 0x6B17D1F2
};

static const uint32_t P256_GY_W[8] = {
    0x37BF51F5, 0xCBB64068, 0x6B315ECE, 0x2BCE3357,
    0x7C0CE4D0, 0x6C24416F, 0x5502F25D, 0x4FE342E2
};

static void load_const_bn(BN *dst, const uint32_t *words, size_t len) {
    bn_zero(dst);
    for (size_t i = 0; i < len; i++) dst->w[i] = words[i];
    dst->len = len;
    bn_clamp(dst);
}

void jac_init(JacPoint *pt) {
    if (!pt) return;
    bn_zero(&pt->X);
    bn_zero(&pt->Y);
    bn_zero(&pt->Z);
}

/* Udvostručivanje točke na Jacobian krivulji (P-256 a = -3): R = 2*P */
void point_double_jac(JacPoint *R, const JacPoint *P, const BN *p) {
    if (bn_is_zero(&P->Z)) {
        jac_init(R);
        return;
    }

    BN S, M, Z2, T1, T2;

    /* Z2 = Z^2 mod p */
    bn_mod_mul(&Z2, &P->Z, &P->Z, p);

    /* M = 3*(X - Z2)*(X + Z2) mod p */
    bn_mod_sub(&T1, &P->X, &Z2, p);
    bn_mod_add(&T2, &P->X, &Z2, p);
    bn_mod_mul(&M, &T1, &T2, p);
    
    BN three;
    bn_zero(&three); three.w[0] = 3; three.len = 1;
    bn_mod_mul(&M, &M, &three, p);

    /* S = 4*X*Y^2 mod p */
    bn_mod_mul(&T1, &P->Y, &P->Y, p);
    bn_mod_mul(&S, &P->X, &T1, p);
    BN four;
    bn_zero(&four); four.w[0] = 4; four.len = 1;
    bn_mod_mul(&S, &S, &four, p);

    /* R.X = M^2 - 2*S mod p */
    bn_mod_mul(&R->X, &M, &M, p);
    bn_mod_add(&T1, &S, &S, p);
    bn_mod_sub(&R->X, &R->X, &T1, p);

    /* R.Y = M*(S - R.X) - 8*Y^4 mod p */
    bn_mod_sub(&T1, &S, &R->X, p);
    bn_mod_mul(&R->Y, &M, &T1, p);
    
    bn_mod_mul(&T2, &P->Y, &P->Y, p);
    bn_mod_mul(&T2, &T2, &T2, p);
    BN eight;
    bn_zero(&eight); eight.w[0] = 8; eight.len = 1;
    bn_mod_mul(&T2, &T2, &eight, p);
    bn_mod_sub(&R->Y, &R->Y, &T2, p);

    /* R.Z = 2*Y*Z mod p */
    bn_mod_mul(&R->Z, &P->Y, &P->Z, p);
    bn_mod_add(&R->Z, &R->Z, &R->Z, p);
}

/* Zbrajanje dviju Jacobian točaka: R = P + Q */
void point_add_jac(JacPoint *R, const JacPoint *P, const JacPoint *Q, const BN *p) {
    if (bn_is_zero(&P->Z)) { *R = *Q; return; }
    if (bn_is_zero(&Q->Z)) { *R = *P; return; }

    BN Z1_2, Z2_2, U1, U2, S1, S2, H, r_val, H2, H3, T;

    bn_mod_mul(&Z1_2, &P->Z, &P->Z, p);
    bn_mod_mul(&Z2_2, &Q->Z, &Q->Z, p);

    bn_mod_mul(&U1, &P->X, &Z2_2, p);
    bn_mod_mul(&U2, &Q->X, &Z1_2, p);

    bn_mod_mul(&S1, &P->Y, &Q->Z, p);
    bn_mod_mul(&S1, &S1, &Z2_2, p);

    bn_mod_mul(&S2, &Q->Y, &P->Z, p);
    bn_mod_mul(&S2, &S2, &Z1_2, p);

    if (bn_cmp(&U1, &U2) == 0) {
        if (bn_cmp(&S1, &S2) == 0) {
            point_double_jac(R, P, p);
            return;
        } else {
            jac_init(R);
            return;
        }
    }

    bn_mod_sub(&H, &U2, &U1, p);
    bn_mod_sub(&r_val, &S2, &S1, p);

    bn_mod_mul(&H2, &H, &H, p);
    bn_mod_mul(&H3, &H2, &H, p);

    /* R.X = r^2 - H^3 - 2*U1*H^2 mod p */
    bn_mod_mul(&R->X, &r_val, &r_val, p);
    bn_mod_sub(&R->X, &R->X, &H3, p);
    bn_mod_mul(&T, &U1, &H2, p);
    bn_mod_add(&T, &T, &T, p);
    bn_mod_sub(&R->X, &R->X, &T, p);

    /* R.Y = r*(U1*H^2 - R.X) - S1*H^3 mod p */
    bn_mod_mul(&T, &U1, &H2, p);
    bn_mod_sub(&T, &T, &R->X, p);
    bn_mod_mul(&R->Y, &r_val, &T, p);
    bn_mod_mul(&T, &S1, &H3, p);
    bn_mod_sub(&R->Y, &R->Y, &T, p);

    /* R.Z = Z1*Z2*H mod p */
    bn_mod_mul(&R->Z, &P->Z, &Q->Z, p);
    bn_mod_mul(&R->Z, &R->Z, &H, p);
}

/* Skalarno množenje: R = k * P (Double-and-Add metoda) */
static void point_mul(JacPoint *R, const BN *k, const JacPoint *P, const BN *p) {
    JacPoint Res, Base;
    jac_init(&Res);
    Base = *P;

    for (size_t i = 0; i < k->len * 32; i++) {
        size_t word_idx = i / 32;
        size_t bit_idx = i % 32;
        if ((k->w[word_idx] >> bit_idx) & 1) {
            point_add_jac(&Res, &Res, &Base, p);
        }
        point_double_jac(&Base, &Base, p);
    }
    *R = Res;
}

int ecdsa_p256_verify(const ECDSA_P256_PublicKey *pubkey,
                      const uint8_t *hash, size_t hash_len,
                      const BN *r, const BN *s) {
    if (!pubkey || !hash || hash_len == 0 || !r || !s) return 0;

    BN p, n, e, w, u1, u2;
    load_const_bn(&p, P256_P_W, 8);
    load_const_bn(&n, P256_N_W, 8);

    if (bn_is_zero(r) || bn_cmp(r, &n) >= 0) return 0;
    if (bn_is_zero(s) || bn_cmp(s, &n) >= 0) return 0;

    /* e = hash mod n (uzimamo prvih 32 bajta) */
    bn_zero(&e);
    size_t bytes_to_take = (hash_len > 32) ? 32 : hash_len;
    for (size_t i = 0; i < bytes_to_take; i++) {
        uint8_t byte_val = hash[bytes_to_take - 1 - i];
        e.w[i / 4] |= ((uint32_t)byte_val) << ((i % 4) * 8);
    }
    e.len = (bytes_to_take + 3) / 4;
    bn_clamp(&e);
    bn_mod(&e, &e, &n);

    /* w = s^-1 mod n */
    bn_mod_inv(&w, s, &n);

    /* u1 = e*w mod n, u2 = r*w mod n */
    bn_mod_mul(&u1, &e, &w, &n);
    bn_mod_mul(&u2, r, &w, &n);

    /* G točka */
    JacPoint G_jac;
    load_const_bn(&G_jac.X, P256_GX_W, 8);
    load_const_bn(&G_jac.Y, P256_GY_W, 8);
    bn_zero(&G_jac.Z); G_jac.Z.w[0] = 1; G_jac.Z.len = 1;

    /* Q točka */
    JacPoint Q_jac;
    Q_jac.X = pubkey->Q.x;
    Q_jac.Y = pubkey->Q.y;
    bn_zero(&Q_jac.Z); Q_jac.Z.w[0] = 1; Q_jac.Z.len = 1;

    /* R1 = u1 * G, R2 = u2 * Q */
    JacPoint R1, R2, R_total;
    point_mul(&R1, &u1, &G_jac, &p);
    point_mul(&R2, &u2, &Q_jac, &p);

    /* R_total = R1 + R2 */
    point_add_jac(&R_total, &R1, &R2, &p);

    if (bn_is_zero(&R_total.Z)) return 0;

    /* Prevođenje iz Jacobian u Affine: X_aff = X / Z^2 mod p */
    BN Z2, Z2_inv, X_aff;
    bn_mod_mul(&Z2, &R_total.Z, &R_total.Z, &p);
    bn_mod_inv(&Z2_inv, &Z2, &p);
    bn_mod_mul(&X_aff, &R_total.X, &Z2_inv, &p);

    /* v = X_aff mod n */
    BN v;
    bn_mod(&v, &X_aff, &n);

    return (bn_cmp(&v, r) == 0);
}
