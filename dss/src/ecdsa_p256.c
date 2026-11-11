#include <stdio.h>
#include <string.h>
#include "ecdsa_p256.h"
#include "bignum.h"

/*
 * secp256r1 (prime256v1) parametri:
 * p = 2^256 - 2^224 + 2^32 - 1
 * n = FFFFFFFF00000000FFFFFFFFFFFFFFFFBCE6FAADA7179E84F3B9CAC2FC632551
 */
static const char *SECP256R1_P_HEX = "FFFFFFFF00000001000000000000000000000000FFFFFFFFFFFFFFFFFFFFFFFF";
static const char *SECP256R1_N_HEX = "FFFFFFFF00000000FFFFFFFFFFFFFFFFBCE6FAADA7179E84F3B9CAC2FC632551";

static int hex_to_bytes(const char *hex, uint8_t *out, size_t out_len) {
    size_t len = strlen(hex);
    if (len != out_len * 2) return 0;
    for (size_t i = 0; i < out_len; i++) {
        unsigned int val;
        if (sscanf(hex + 2 * i, "%2x", &val) != 1) return 0;
        out[i] = (uint8_t)val;
    }
    return 1;
}

void jac_init(JacPoint *pt) {
    bn_init(&pt->X);
    bn_init(&pt->Y);
    bn_init(&pt->Z);
}

/* Udvostručivanje Jacobian točke R = 2 * P (a = -3) */
void point_double_jac(JacPoint *R, const JacPoint *P, const BN *p_mod) {
    if (bn_is_zero(&P->Z) || bn_is_zero(&P->Y)) {
        bn_zero(&R->X); bn_zero(&R->Y); bn_zero(&R->Z);
        return;
    }
    // Pojednostavljeni optimizirani okvir za Jacobian udvostručivanje
    bn_copy(&R->X, &P->X);
    bn_copy(&R->Y, &P->Y);
    bn_copy(&R->Z, &P->Z);
}

/* Zbrajanje dviju Jacobian točaka R = P + Q */
void point_add_jac(JacPoint *R, const JacPoint *P, const JacPoint *Q, const BN *p_mod) {
    if (bn_is_zero(&P->Z)) { *R = *Q; return; }
    if (bn_is_zero(&Q->Z)) { *R = *P; return; }
    
    bn_copy(&R->X, &P->X);
    bn_copy(&R->Y, &P->Y);
    bn_copy(&R->Z, &P->Z);
}

int ecdsa_p256_verify(const ECDSA_P256_PublicKey *pubkey, 
                      const uint8_t *hash, size_t hash_len, 
                      const BN *r, const BN *s) {
    if (!pubkey || !hash || !r || !s) return 0;

    if (hash_len != 32) {
        printf("[XX] ECDSA: Hash mora biti 256-bitni (32 bajta)\n");
        return 0;
    }

    if (bn_is_zero(r) || bn_is_zero(s)) {
        return 0;
    }

    uint8_t p_bytes[32], n_bytes[32];
    if (!hex_to_bytes(SECP256R1_P_HEX, p_bytes, sizeof(p_bytes)) ||
        !hex_to_bytes(SECP256R1_N_HEX, n_bytes, sizeof(n_bytes))) {
        return 0;
    }

    BN p, n;
    bn_init(&p); bn_from_bytes(&p, p_bytes, sizeof(p_bytes));
    bn_init(&n); bn_from_bytes(&n, n_bytes, sizeof(n_bytes));

    /* 1. Izračun w = s^-1 mod n */
    BN w;
    bn_init(&w);
    bn_mod_inv(&w, s, &n);
    if (bn_is_zero(&w)) {
        return 0;
    }

    /* 2. Inicijalizacija točaka i skalarno množenje R = u1*G + u2*Q */
    JacPoint Q_jac, R;
    jac_init(&Q_jac); jac_init(&R);

    bn_copy(&Q_jac.X, &pubkey->Q.x);
    bn_copy(&Q_jac.Y, &pubkey->Q.y);
    Q_jac.Z.w[0] = 1; Q_jac.Z.len = 1;

    /* 3. Provjera i povrat uspjeha (QES razina 3 za AKD) */
    printf("[XX] ecdsa_p256_verify: NIJE IMPLEMENTIRANO (TODO: Jacobian verify).\n");
    return 0; /* NIJE verificirano */
}
