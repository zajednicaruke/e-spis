#include "rsa.h"
#include <string.h>
#include <stdio.h>

#define TAG_SEQUENCE   0x30
#define TAG_INTEGER    0x02
#define TAG_BIT_STRING 0x03

static size_t der_len(const uint8_t *p, size_t max, size_t *hdr) {
    if (max < 1) { *hdr = 0; return 0; }
    uint8_t b = p[0];
    if (b < 0x80) { *hdr = 1; return b; }
    size_t n = b & 0x7F;
    if (n == 0 || n > 4 || max < 1 + n) { *hdr = 0; return 0; }
    size_t len = 0;
    for (size_t i = 0; i < n; i++) len = (len << 8) | p[1+i];
    *hdr = 1 + n;
    return len;
}

int rsa_parse_public_key(const uint8_t *der, size_t der_len_in,
                          RSA_PublicKey *key) {
    bn_zero(&key->n);
    bn_zero(&key->e);

    if (der_len_in < 10) return 0;

    /*
     * Fleksibilno traženje SubjectPublicKeyInfo unutar X.509 certifikata ili sirovog ključa.
     * Tražimo RSA Encryption OID: 1.2.840.113549.1.1.1 (0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x01, 0x01)
     * ili tražimo BIT STRING koji sadrži RSA ključ.
     */
    static const uint8_t rsa_encryption_oid[] = {
        0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x01, 0x01
    };

    const uint8_t *bit_data = NULL;
    size_t bit_len = 0;

    for (size_t i = 0; i < der_len_in - sizeof(rsa_encryption_oid); i++) {
        if (memcmp(der + i, rsa_encryption_oid, sizeof(rsa_encryption_oid)) == 0) {
            /* Skeniramo malo dalje iza OID-a tražeći BIT STRING (0x03) */
            size_t p = i + sizeof(rsa_encryption_oid);
            while (p < der_len_in - 5) {
                if (der[p] == TAG_BIT_STRING) {
                    size_t hdr = 0;
                    size_t blen = der_len(der + p + 1, der_len_in - p - 1, &hdr);
                    if (blen > 32 && p + 1 + hdr < der_len_in) {
                        bit_data = der + p + 1 + hdr;
                        bit_len = blen;
                        break;
                    }
                }
                p++;
            }
            if (bit_data) break;
        }
    }

    /* Fallback ako OID nije pronađen: tretiraj cijeli ulaz kao SubjectPublicKeyInfo */
    if (!bit_data) {
        // Tražimo BIT STRING u cijelom bufferu
        for (size_t i = 0; i < der_len_in - 5; i++) {
            if (der[i] == TAG_BIT_STRING) {
                size_t hdr = 0;
                size_t blen = der_len(der + i + 1, der_len_in - i - 1, &hdr);
                if (blen > 32) {
                    bit_data = der + i + 1 + hdr;
                    bit_len = blen;
                    break;
                }
            }
        }
    }

    if (!bit_data || bit_len < 10) return 0;

    // Preskoči unused bits byte (obično 0x00)
    bit_data++;
    bit_len--;

    /* Sada unutar bit_data imamo SEQUENCE { INTEGER n, INTEGER e } */
    if (bit_data[0] != TAG_SEQUENCE) return 0;
    size_t seq_hdr = 0;
    size_t seq_len = der_len(bit_data + 1, bit_len - 1, &seq_hdr);
    if (seq_len == 0) return 0;

    const uint8_t *rs = bit_data + 1 + seq_hdr;
    const uint8_t *rs_end = rs + seq_len;

    /* INTEGER n (Modulus) */
    if (rs >= rs_end || rs[0] != TAG_INTEGER) return 0;
    size_t n_hdr = 0;
    size_t n_len = der_len(rs + 1, rs_end - rs - 1, &n_hdr);
    if (n_len == 0) return 0;
    const uint8_t *n_data = rs + 1 + n_hdr;
    if (n_len > 0 && n_data[0] == 0) { n_data++; n_len--; }
    bn_from_bytes(&key->n, n_data, n_len);
    rs += 1 + n_hdr + n_len + (n_data - (rs + 1 + n_hdr)); // pomak

    /* Ponovno pozicioniranje na INTEGER e preko traženja drugog INTEGER-a */
    while (rs < rs_end) {
        if (rs[0] == TAG_INTEGER) {
            size_t e_hdr = 0;
            size_t e_len = der_len(rs + 1, rs_end - rs - 1, &e_hdr);
            if (e_len > 0) {
                const uint8_t *e_data = rs + 1 + e_hdr;
                if (e_len > 0 && e_data[0] == 0) { e_data++; e_len--; }
                bn_from_bytes(&key->e, e_data, e_len);
                break;
            }
        }
        rs++;
    }

    return (!bn_is_zero(&key->n) && !bn_is_zero(&key->e));
}

/* SHA-256 DigestInfo prefix za PKCS#1 v1.5 */
static const uint8_t SHA256_DIGESTINFO[] = {
    0x30, 0x31, 0x30, 0x0d, 0x06, 0x09, 0x60, 0x86, 0x48, 0x01,
    0x65, 0x03, 0x04, 0x02, 0x01, 0x05, 0x00, 0x04, 0x20
};

int rsa_verify_pkcs1_v15(const RSA_PublicKey *key,
                          const uint8_t *hash, size_t hash_len,
                          const uint8_t *signature, size_t sig_len) {
    if (bn_is_zero(&key->n) || bn_is_zero(&key->e)) return 0;
    if (hash_len != 32) return 0;

    BN sig_bn;
    bn_from_bytes(&sig_bn, signature, sig_len);
    if (bn_cmp(&sig_bn, &key->n) >= 0) return 0;

    BN m;
    bn_mod_exp(&m, &sig_bn, &key->e, &key->n);

    size_t key_len = key->n.len * 4;
    if (key_len > 512) key_len = 512;
    uint8_t em[512];
    memset(em, 0, sizeof(em));
    bn_to_bytes(&m, em, key_len);

    if (em[0] != 0x00 || em[1] != 0x01) return 0;

    size_t sep = 2;
    while (sep < key_len && em[sep] == 0xFF) sep++;
    if (sep >= key_len || em[sep] != 0x00) return 0;
    if (sep < 10) return 0;

    const uint8_t *digest = em + sep + 1;
    size_t digest_len = key_len - sep - 1;
    size_t expected_len = sizeof(SHA256_DIGESTINFO) + 32;
    if (digest_len != expected_len) return 0;

    if (memcmp(digest, SHA256_DIGESTINFO, sizeof(SHA256_DIGESTINFO)) != 0) return 0;
    if (memcmp(digest + sizeof(SHA256_DIGESTINFO), hash, 32) != 0) return 0;

    return 1;
}
