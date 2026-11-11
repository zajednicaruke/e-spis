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

    if (der_len_in < 4 || der[0] != TAG_SEQUENCE) return 0;
    size_t hdr = 0;
    size_t seq_len = der_len(der + 1, der_len_in - 1, &hdr);
    if (seq_len == 0) return 0;
    const uint8_t *p = der + 1 + hdr;
    const uint8_t *end = p + seq_len;
    if (end > der + der_len_in) end = der + der_len_in;

    /* AlgorithmIdentifier */
    if (p >= end || p[0] != TAG_SEQUENCE) return 0;
    size_t alg_hdr = 0;
    size_t alg_len = der_len(p + 1, end - p - 1, &alg_hdr);
    if (alg_len == 0) return 0;
    p += 1 + alg_hdr + alg_len;

    /* BIT STRING */
    if (p >= end || p[0] != TAG_BIT_STRING) return 0;
    size_t bit_hdr = 0;
    size_t bit_len = der_len(p + 1, end - p - 1, &bit_hdr);
    if (bit_len == 0) return 0;
    const uint8_t *bit_data = p + 1 + bit_hdr;
    if (bit_data >= end) return 0;
    bit_data++;
    bit_len--;

    /* SEQUENCE { INTEGER n, INTEGER e } */
    if (bit_len < 4 || bit_data[0] != TAG_SEQUENCE) return 0;
    size_t rs_hdr = 0;
    size_t rs_len = der_len(bit_data + 1, bit_len - 1, &rs_hdr);
    if (rs_len == 0) return 0;
    const uint8_t *rs = bit_data + 1 + rs_hdr;
    const uint8_t *rs_end = rs + rs_len;

    /* INTEGER n */
    if (rs >= rs_end || rs[0] != TAG_INTEGER) return 0;
    size_t n_hdr = 0;
    size_t n_len = der_len(rs + 1, rs_end - rs - 1, &n_hdr);
    if (n_len == 0) return 0;
    size_t n_total = n_len;  /* originalna duljina za pomicanje */
    const uint8_t *n_data = rs + 1 + n_hdr;
    if (n_len > 0 && n_data[0] == 0) { n_data++; n_len--; }
    bn_from_bytes(&key->n, n_data, n_len);
    rs += 1 + n_hdr + n_total;

    /* INTEGER e */
    if (rs >= rs_end || rs[0] != TAG_INTEGER) return 0;
    size_t e_hdr = 0;
    size_t e_len = der_len(rs + 1, rs_end - rs - 1, &e_hdr);
    if (e_len == 0) return 0;
    size_t e_total = e_len;
    const uint8_t *e_data = rs + 1 + e_hdr;
    if (e_len > 0 && e_data[0] == 0) { e_data++; e_len--; }
    bn_from_bytes(&key->e, e_data, e_len);
    (void)e_total;

    return 1;
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

    /* sig^e mod n */
    BN sig_bn;
    bn_from_bytes(&sig_bn, signature, sig_len);
    if (bn_cmp(&sig_bn, &key->n) >= 0) return 0;

    BN m;
    bn_mod_exp(&m, &sig_bn, &key->e, &key->n);

    /* Pretvori m u bajtove */
    size_t key_len = key->n.len * 4;
    if (key_len > 512) key_len = 512;
    uint8_t em[512];
    memset(em, 0, sizeof(em));
    bn_to_bytes(&m, em, key_len);

    /* Provjeri PKCS#1 v1.5 padding:
       0x00 0x01 0xFF... 0x00 DigestInfo SHA256(hash) */
    if (em[0] != 0x00 || em[1] != 0x01) return 0;

    size_t sep = 2;
    while (sep < key_len && em[sep] == 0xFF) sep++;
    if (sep >= key_len || em[sep] != 0x00) return 0;
    if (sep < 10) return 0;  /* minimalno 8 0xFF bajtova */

    const uint8_t *digest = em + sep + 1;
    size_t digest_len = key_len - sep - 1;
    size_t expected_len = sizeof(SHA256_DIGESTINFO) + 32;
    if (digest_len != expected_len) return 0;

    if (memcmp(digest, SHA256_DIGESTINFO, sizeof(SHA256_DIGESTINFO)) != 0) return 0;
    if (memcmp(digest + sizeof(SHA256_DIGESTINFO), hash, 32) != 0) return 0;

    return 1;
}
