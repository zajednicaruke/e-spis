#ifndef RSA_H
#define RSA_H
#include "bignum.h"
#include <stdint.h>
#include <stddef.h>

typedef struct {
    BN n;   /* modulus */
    BN e;   /* public exponent */
} RSA_PublicKey;

/* Parsiraj RSA public key iz DER SubjectPublicKeyInfo */
int rsa_parse_public_key(const uint8_t *der, size_t der_len, RSA_PublicKey *key);

/* RSA verify PKCS#1 v1.5 (SHA-256) */
int rsa_verify_pkcs1_v15(const RSA_PublicKey *key,
                          const uint8_t *hash, size_t hash_len,
                          const uint8_t *signature, size_t sig_len);

#endif
