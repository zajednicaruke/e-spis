#ifndef ECDSA_P256_H
#define ECDSA_P256_H

#include <stdint.h>
#include <stddef.h>
#include "bignum.h"

/* Jacobian točka (X, Y, Z) nad P-256 */
typedef struct {
    BN X;
    BN Y;
    BN Z;
} JacPoint;

/* Affine točka (x, y) */
typedef struct {
    BN x;
    BN y;
} AffinePoint;

/* ECDSA P-256 javni ključ */
typedef struct {
    AffinePoint Q;
} ECDSA_P256_PublicKey;

/* Inicijalizacija Jacobian točke */
void jac_init(JacPoint *pt);

/* Udvostručivanje Jacobian točke: R = 2 * P */
void point_double_jac(JacPoint *R, const JacPoint *P, const BN *p);

/* Zbrajanje dviju Jacobian točaka: R = P + Q */
void point_add_jac(JacPoint *R, const JacPoint *P, const JacPoint *Q, const BN *p);

/* Verifikacija ECDSA P-256 potpisa (r, s) nad 32-bitnim hashom */
int ecdsa_p256_verify(const ECDSA_P256_PublicKey *pubkey, 
                      const uint8_t *hash, size_t hash_len, 
                      const BN *r, const BN *s);

#endif // ECDSA_P256_H
