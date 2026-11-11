#include "der.h"
#include "util.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>

/* ============================================================
 * ASN.1 / DER tagovi
 * ============================================================ */
#define TAG_INTEGER           0x02
#define TAG_BIT_STRING        0x03
#define TAG_OCTET_STRING      0x04
#define TAG_NULL              0x05
#define TAG_OID               0x06
#define TAG_UTF8_STRING       0x0C




#define TAG_SEQUENCE          0x30
#define TAG_SET               0x31
#define TAG_PRINTABLE_STRING  0x13
#define TAG_T61_STRING        0x14
#define TAG_IA5_STRING        0x16
#define TAG_UTC_TIME          0x17
#define TAG_GENERALIZED_TIME  0x18
#define TAG_BMP_STRING        0x1E

/* OID-ovi (2.5.4.x) — DER encoded */
static const uint8_t OID_CN[]  = {0x55, 0x04, 0x03};
static const uint8_t OID_O[]   = {0x55, 0x04, 0x0A};
static const uint8_t OID_OU[]  = {0x55, 0x04, 0x0B};
static const uint8_t OID_C[]   = {0x55, 0x04, 0x06};
static const uint8_t OID_SN[]  = {0x55, 0x04, 0x05};

/* ============================================================
 * Niski nivo: čitanje DER elemenata
 * ============================================================ */

static size_t der_citaj_length(const unsigned char *p, size_t max,
                                size_t *out_hdr_len) {
    if (max < 1) { *out_hdr_len = 0; return 0; }
    unsigned char b = p[0];
    if (b < 0x80) { *out_hdr_len = 1; return b; }
    size_t n = b & 0x7F;
    if (n == 0 || n > 4 || max < 1 + n) { *out_hdr_len = 0; return 0; }
    size_t len = 0;
    for (size_t i = 0; i < n; i++) len = (len << 8) | p[1 + i];
    *out_hdr_len = 1 + n;
    return len;
}

static const unsigned char *der_skip(const unsigned char *p, size_t max,
                                      size_t *consumed) {
    if (max < 2) { *consumed = 0; return NULL; }
    size_t hdr = 0;
    size_t len = der_citaj_length(p + 1, max - 1, &hdr);
    size_t total = 1 + hdr + len;
    if (total > max) { *consumed = 0; return NULL; }
    *consumed = total;
    return p + total;
}

/* ============================================================
 * OID bajtovi -> string (npr. "2.5.4.3")
 * ============================================================ */
static int der_oid_to_string(const uint8_t *oid, size_t oid_len,
                              char *out, size_t outlen) {
    if (oid_len < 1 || outlen < 8) return 0;
    int n = 0;
    unsigned int v = oid[0];
    n += snprintf(out + n, outlen - n, "%u.%u", v / 40, v % 40);
    unsigned long val = 0;
    for (size_t i = 1; i < oid_len; i++) {
        val = (val << 7) | (oid[i] & 0x7F);
        if (!(oid[i] & 0x80)) {
            n += snprintf(out + n, outlen - n, ".%lu", val);
            val = 0;
            if ((size_t)n >= outlen - 1) break;
        }
    }
    return n;
}

/* ============================================================
 * ASN.1 String -> C string (UTF8, Printable, IA5, T61, BMP)
 * ============================================================ */
static int der_string_value(const unsigned char *val, size_t len,
                             unsigned char tag, char *out, size_t outlen) {
    if (outlen == 0) return 0;
    size_t i = 0;
    if (tag == TAG_BMP_STRING) {
        for (size_t j = 0; j < len && i + 1 < outlen; j += 2) {
            if (val[j] == 0x00) out[i++] = (char)val[j+1];
            else out[i++] = '?';
        }
    } else {
        for (size_t j = 0; j < len && i + 1 < outlen; j++) {
            unsigned char c = val[j];
            if (c < 0x80) out[i++] = (char)c;
            else if (c >= 0xC0) out[i++] = (char)c;
        }
    }
    out[i] = '\0';
    return (int)i;
}

/* ============================================================
 * Formatiraj ASN.1 Time (UTCTime / GeneralizedTime) u dd.mm.yyyy HH:MM:SS
 * ============================================================ */
static int der_time_value(const unsigned char *val, size_t len,
                           unsigned char tag, char *out, size_t outlen) {
    if (outlen < 20) return 0;
    char s[32] = {0};
    size_t copy = len < 30 ? len : 30;
    for (size_t i = 0; i < copy; i++) {
        if (val[i] >= '0' && val[i] <= '9') s[i] = val[i];
        else { copy = i; break; }
    }
    int Y, M, D, h, m, sec;
    if (tag == TAG_UTC_TIME && copy >= 12) {
        int yy = (s[0]-'0')*10 + (s[1]-'0');
        Y = (yy < 50) ? 2000 + yy : 1900 + yy;
        M = (s[2]-'0')*10 + (s[3]-'0');
        D = (s[4]-'0')*10 + (s[5]-'0');
        h = (s[6]-'0')*10 + (s[7]-'0');
        m = (s[8]-'0')*10 + (s[9]-'0');
        sec = (s[10]-'0')*10 + (s[11]-'0');
    } else if (tag == TAG_GENERALIZED_TIME && copy >= 14) {
        Y = (s[0]-'0')*1000 + (s[1]-'0')*100 + (s[2]-'0')*10 + (s[3]-'0');
        M = (s[4]-'0')*10 + (s[5]-'0');
        D = (s[6]-'0')*10 + (s[7]-'0');
        h = (s[8]-'0')*10 + (s[9]-'0');
        m = (s[10]-'0')*10 + (s[11]-'0');
        sec = (s[12]-'0')*10 + (s[13]-'0');
    } else {
        snprintf(out, outlen, "(neispravan format)");
        return 0;
    }
    snprintf(out, outlen, "%02d.%02d.%04d %02d:%02d:%02d", D, M, Y, h, m, sec);
    return 1;
}

/* ============================================================
 * Parsiraj RDN: SET { SEQUENCE { OID, VALUE } }
 * ============================================================ */
static void der_parsiraj_rdn(const unsigned char *p, size_t len, DER_Cert *c) {
    if (len < 2 || p[0] != TAG_SEQUENCE) return;
    size_t hdr = 0;
    size_t inner = der_citaj_length(p + 1, len - 1, &hdr);
    if (inner == 0) return;
    const unsigned char *q = p + 1 + hdr;
    const unsigned char *end = q + inner;
    if (end > p + len) end = p + len;

    while (q + 2 < end) {
        if (q[0] != TAG_SET) { q++; continue; }
        size_t shdr = 0;
        size_t slen = der_citaj_length(q + 1, end - q - 1, &shdr);
        if (slen == 0) break;
        const unsigned char *s = q + 1 + shdr;
        const unsigned char *send = s + slen;
        if (send > end) send = end;

        while (s + 2 < send) {
            if (s[0] != TAG_SEQUENCE) { s++; continue; }
            size_t sehdr = 0;
            size_t selen = der_citaj_length(s + 1, send - s - 1, &sehdr);
            if (selen == 0) break;
            const unsigned char *se = s + 1 + sehdr;
            const unsigned char *seend = se + selen;
            if (seend > send) seend = send;

            if (se + 2 < seend && se[0] == TAG_OID) {
                size_t ohdr = 0;
                size_t olen = der_citaj_length(se + 1, seend - se - 1, &ohdr);
                if (olen > 0 && olen <= 8 && se + 1 + ohdr + olen <= seend) {
                    const uint8_t *oid = se + 1 + ohdr;
                    const unsigned char *val = se + 1 + ohdr + olen;
                    if (val + 2 <= seend) {
                        unsigned char vtag = val[0];
                        size_t vhdr = 0;
                        size_t vlen = der_citaj_length(val + 1,
                                       seend - val - 1, &vhdr);
                        if (vlen > 0 && val + 1 + vhdr + vlen <= seend) {
                            const unsigned char *vdata = val + 1 + vhdr;

                            if (olen == sizeof(OID_CN) && memcmp(oid, OID_CN, olen) == 0) {
                                der_string_value(vdata, vlen, vtag, c->cn, sizeof(c->cn));
                            } else if (olen == sizeof(OID_O) && memcmp(oid, OID_O, olen) == 0) {
                                der_string_value(vdata, vlen, vtag, c->o, sizeof(c->o));
                            } else if (olen == sizeof(OID_OU) && memcmp(oid, OID_OU, olen) == 0) {
                                der_string_value(vdata, vlen, vtag, c->ou, sizeof(c->ou));
                            } else if (olen == sizeof(OID_C) && memcmp(oid, OID_C, olen) == 0) {
                                der_string_value(vdata, vlen, vtag, c->c, sizeof(c->c));
                            } else if (olen == sizeof(OID_SN) && memcmp(oid, OID_SN, olen) == 0) {
                                der_string_value(vdata, vlen, vtag, c->oib, sizeof(c->oib));
                            }
                        }
                    }
                }
            }
            s = se + 1 + sehdr + selen;
        }
        q = s;
        if (slen == 0) break;
    }
}

/* ============================================================
 * Parsiraj X.509 certifikat
 *   Certificate ::= SEQUENCE {
 *       tbsCertificate       SEQUENCE {
 *           version [0] EXPLICIT (optional),
 *           serialNumber INTEGER,
 *           signature AlgorithmIdentifier,
 *           issuer Name,
 *           validity SEQUENCE { notBefore Time, notAfter Time },
 *           subject Name,
 *           ...
 *       },
 *       signatureAlgorithm AlgorithmIdentifier,
 *       signatureValue BIT STRING
 *   }
 * ============================================================ */
static int der_parsiraj_x509(const unsigned char *cert, size_t cert_len,
                              DER_Cert *out) {
    memset(out, 0, sizeof(*out));
    out->der_velicina = cert_len;

    if (cert_len < 4 || cert[0] != TAG_SEQUENCE) return 0;

    size_t hdr = 0;
    size_t seqlen = der_citaj_length(cert + 1, cert_len - 1, &hdr);
    if (seqlen == 0) return 0;
    const unsigned char *p = cert + 1 + hdr;
    const unsigned char *end = p + seqlen;
    if (end > cert + cert_len) end = cert + cert_len;

    /* tbsCertificate */
    if (p + 2 > end || p[0] != TAG_SEQUENCE) return 0;
    size_t thdr = 0;
    size_t tlen = der_citaj_length(p + 1, end - p - 1, &thdr);
    if (tlen == 0) return 0;
    const unsigned char *tbs = p + 1 + thdr;
    const unsigned char *tbsend = tbs + tlen;
    if (tbsend > end) tbsend = end;

    const unsigned char *q = tbs;

    /* [0] EXPLICIT Version */
    if (q < tbsend && q[0] == 0xA0) {
        size_t skipped = 0;
        q = der_skip(q, tbsend - q, &skipped);
        if (!q) return 0;
    }

    /* serialNumber */
    if (q < tbsend && q[0] == TAG_INTEGER) {
        size_t ihdr = 0;
        size_t ilen = der_citaj_length(q + 1, tbsend - q - 1, &ihdr);
        if (ilen > 0 && q + 1 + ihdr + ilen <= tbsend) {
            const unsigned char *ip = q + 1 + ihdr;
            int n = 0;
            for (size_t i = 0; i < ilen && n < 60; i++) {
                n += snprintf(out->serial + n, sizeof(out->serial) - n,
                              "%02X", ip[i]);
            }
        }
        q += 1 + ihdr + ilen;
    }

    /* signature AlgorithmIdentifier */
    if (q < tbsend && q[0] == TAG_SEQUENCE) {
        size_t shdr = 0;
        size_t slen = der_citaj_length(q + 1, tbsend - q - 1, &shdr);
        if (slen > 0 && q + 1 + shdr + slen <= tbsend) {
            const unsigned char *alg = q + 1 + shdr;
            if (alg < q + 1 + shdr + slen && alg[0] == TAG_OID) {
                size_t ahdr = 0;
                size_t alen = der_citaj_length(alg + 1,
                               (q + 1 + shdr + slen) - alg - 1, &ahdr);
                if (alen > 0 && alen < 16 && alg + 1 + ahdr + alen <= q + 1 + shdr + slen) {
                    der_oid_to_string(alg + 1 + ahdr, alen,
                                      out->algoritam, sizeof(out->algoritam));
                }
            }
        }
        q += 1 + shdr + slen;
    }

    /* issuer */
    if (q < tbsend && q[0] == TAG_SEQUENCE) {
        size_t ihdr = 0;
        size_t ilen = der_citaj_length(q + 1, tbsend - q - 1, &ihdr);
        if (ilen > 0 && q + 1 + ihdr + ilen <= tbsend) {
            DER_Cert tmp = {0};
            der_parsiraj_rdn(q, 1 + ihdr + ilen, &tmp);
            snprintf(out->izdavatelj_cn, sizeof(out->izdavatelj_cn), "%s", tmp.cn);
            snprintf(out->izdavatelj_o,  sizeof(out->izdavatelj_o),  "%s", tmp.o);
        }
        q += 1 + ihdr + ilen;
    }

    /* validity */
    if (q < tbsend && q[0] == TAG_SEQUENCE) {
        size_t vhdr = 0;
        size_t vlen = der_citaj_length(q + 1, tbsend - q - 1, &vhdr);
        if (vlen > 0 && q + 1 + vhdr + vlen <= tbsend) {
            const unsigned char *v = q + 1 + vhdr;
            const unsigned char *vend = v + vlen;

            if (v + 2 < vend &&
                (v[0] == TAG_UTC_TIME || v[0] == TAG_GENERALIZED_TIME)) {
                size_t th = 0;
                size_t tl = der_citaj_length(v + 1, vend - v - 1, &th);
                if (tl > 0 && v + 1 + th + tl <= vend) {
                    der_time_value(v + 1 + th, tl, v[0],
                                   out->not_before, sizeof(out->not_before));
                }
                v += 1 + th + tl;
            }
            if (v + 2 < vend &&
                (v[0] == TAG_UTC_TIME || v[0] == TAG_GENERALIZED_TIME)) {
                size_t th = 0;
                size_t tl = der_citaj_length(v + 1, vend - v - 1, &th);
                if (tl > 0 && v + 1 + th + tl <= vend) {
                    der_time_value(v + 1 + th, tl, v[0],
                                   out->not_after, sizeof(out->not_after));
                }
            }
        }
        q += 1 + vhdr + vlen;
    }

    /* subject — ovdje su CN, O, OIB */
    if (q < tbsend && q[0] == TAG_SEQUENCE) {
        size_t shdr = 0;
        size_t slen = der_citaj_length(q + 1, tbsend - q - 1, &shdr);
        if (slen > 0 && q + 1 + shdr + slen <= tbsend) {
            der_parsiraj_rdn(q, 1 + shdr + slen, out);
        }
    }

    out->uspjesno = 1;
    return 1;
}

/* ============================================================
 * Pretvori hex string u DER blob
 * ============================================================ */
static int der_iz_hex_string(const unsigned char *hex, size_t hexlen,
                              unsigned char **out_der, size_t *out_len) {
    if (hexlen < 100) return 0;
    unsigned char *der = malloc(hexlen / 2 + 1);
    if (!der) return 0;
    size_t dl = 0;
    for (size_t i = 0; i + 1 < hexlen; i += 2) {
        unsigned char hi = hex[i], lo = hex[i+1];
        if (!isxdigit(hi) || !isxdigit(lo)) break;
        int hv = isdigit(hi) ? hi-'0' : (hi|32)-'a'+10;
        int lv = isdigit(lo) ? lo-'0' : (lo|32)-'a'+10;
        der[dl++] = (unsigned char)((hv << 4) | lv);
    }
    *out_der = der;
    *out_len = dl;
    return dl > 0;
}

/* ============================================================
 * Iz PDF-a izvuci DER certifikat
 * Strategija: nadji /Contents <hex...> unutar signature dict-a,
 * dekodiraj hex, pa trazi X.509 SEQUENCE (0x30 0x82).
 * ============================================================ */
int der_izvuci_certifikat(const unsigned char *pdf_buf, size_t pdf_len,
                          DER_Cert *out) {
    memset(out, 0, sizeof(*out));

    /* Nadji signature dict marker */
    void *sig = memmem_case(pdf_buf, pdf_len, "/Type/Sig", 9);
    if (!sig) sig = memmem_case(pdf_buf, pdf_len, "/Type /Sig", 10);
    if (!sig) sig = memmem_case(pdf_buf, pdf_len, "/SubFilter", 10);
    if (!sig) return 0;

    const unsigned char *dict_start = (const unsigned char*)sig;
    size_t dict_max = pdf_len - (dict_start - pdf_buf);
    if (dict_max > 100000) dict_max = 100000;

    /* /Contents */
    void *cont = memmem_case(dict_start, dict_max, "/Contents", 9);
    if (!cont) return 0;

    const unsigned char *c = (const unsigned char*)cont + 9;
    const unsigned char *cend = dict_start + dict_max;

    /* Preskoči whitespace */
    while (c < cend && (*c == ' ' || *c == '\n' || *c == '\r' || *c == '\t')) c++;
    if (c >= cend) return 0;

    /* Slučaj A: <hex string> */
    if (*c == '<' && c + 1 < cend && c[1] != '<') {
        c++;
        const unsigned char *hend = memchr(c, '>', cend - c);
        if (!hend) return 0;

        unsigned char *der = NULL;
        size_t dlen = 0;
        if (!der_iz_hex_string(c, hend - c, &der, &dlen)) return 0;

        /* Traži X.509 SEQUENCE: 0x30 0x82 (dugi length) */
        for (size_t off = 0; off + 10 < dlen; off++) {
            if (der[off] == TAG_SEQUENCE && der[off+1] == 0x82) {
                DER_Cert tmp;
                if (der_parsiraj_x509(der + off, dlen - off, &tmp)) {
                    *out = tmp;
                    free(der);
                    return 1;
                }
            }
        }
        free(der);
        return 0;
    }

    /* Slučaj B: (literal string) — rijetko */
    if (*c == '(') {
        const unsigned char *pend = memchr(c, ')', cend - c);
        if (pend && pend - c > 200) {
            for (const unsigned char *q = c + 1; q + 10 < pend; q++) {
                if (q[0] == TAG_SEQUENCE && q[1] == 0x82) {
                    DER_Cert tmp;
                    if (der_parsiraj_x509(q, pend - q, &tmp)) {
                        *out = tmp;
                        return 1;
                    }
                }
            }
        }
    }

    return 0;
}

/* ============================================================
 * Formatiraj DER_Cert u čitljiv string
 * ============================================================ */
void der_formatiraj(const DER_Cert *c, char *out, size_t outlen) {
    if (!c->uspjesno) {
        snprintf(out, outlen, "Certifikat nije parsiran");
        return;
    }
    char buf[1024];
    int n = 0;
    if (c->cn[0])
        n += snprintf(buf + n, sizeof(buf) - n, "CN=%s", c->cn);
    if (c->o[0])
        n += snprintf(buf + n, sizeof(buf) - n, "%sO=%s", n ? " | " : "", c->o);
    if (c->oib[0])
        n += snprintf(buf + n, sizeof(buf) - n, "%sOIB=%s", n ? " | " : "", c->oib);
    if (c->izdavatelj_cn[0])
        n += snprintf(buf + n, sizeof(buf) - n, "%sCA=%s", n ? " | " : "", c->izdavatelj_cn);
    if (c->not_before[0])
        n += snprintf(buf + n, sizeof(buf) - n, "%sValjan od %s", n ? " | " : "", c->not_before);
    if (c->not_after[0])
        n += snprintf(buf + n, sizeof(buf) - n, " do %s", c->not_after);
    if (c->algoritam[0])
        n += snprintf(buf + n, sizeof(buf) - n, "%sAlg=%s", n ? " | " : "", c->algoritam);
    snprintf(out, outlen, "%s", n ? buf : "Certifikat bez prepoznatih polja");
}


/* Javna funkcija: parsiraj raw DER certifikat */
int der_parsiraj_der_cert(const unsigned char *cert, size_t cert_len,
                           DER_Cert *out) {
    return der_parsiraj_x509(cert, cert_len, out);
}
