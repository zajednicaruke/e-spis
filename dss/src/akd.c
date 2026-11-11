#include "akd.h"
#include "util.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

/* ============================================================
 * UTF-16BE code point -> UTF-8
 * ============================================================ */
static int utf16be_to_utf8(unsigned int cp, char *out, size_t outlen) {
    if (cp < 0x80) { if (outlen < 1) return 0; out[0] = (char)cp; return 1; }
    if (cp < 0x800) {
        if (outlen < 2) return 0;
        out[0] = (char)(0xC0 | (cp >> 6));
        out[1] = (char)(0x80 | (cp & 0x3F));
        return 2;
    }
    if (cp >= 0xD800 && cp <= 0xDFFF) return 0;
    if (outlen < 3) return 0;
    out[0] = (char)(0xE0 | (cp >> 12));
    out[1] = (char)(0x80 | ((cp >> 6) & 0x3F));
    out[2] = (char)(0x80 | (cp & 0x3F));
    return 3;
}

/* ============================================================
 * Očisti ime: dopusti slova (uklj. HR dijakritike), brojeve,
 * razmake, točke, crtice. Odbaci CJK, kontrolne, escape.
 * Mapiraj Ŝ (U+015C) -> ć (U+0107) — AKD escape bug.
 * ============================================================ */
static void akd_ocisti(char *s) {
    if (!s || !*s) return;
    char tmp[512];
    size_t tlen = 0;
    size_t i = 0;
    size_t n = strlen(s);
    while (i < n && tlen < sizeof(tmp) - 4) {
        unsigned char c = (unsigned char)s[i];
        if (c < 0x20) { i++; continue; }
        if (c < 0x80) {
            if (isalnum(c) || c == ' ' || c == '-' || c == '.' || c == '\'') {
                tmp[tlen++] = (char)c;
            }
            i++;
        } else if ((c & 0xE0) == 0xC0 && i + 1 < n) {
            unsigned int cp = ((c & 0x1F) << 6) | ((unsigned char)s[i+1] & 0x3F);
            if (cp == 0x015C) {
                tmp[tlen++] = (char)0xC4;
                tmp[tlen++] = (char)0x87;
            } else if (cp >= 0x0100 && cp <= 0x017F) {
                tmp[tlen++] = (char)c;
                tmp[tlen++] = (char)s[i+1];
            }
            i += 2;
        } else if ((c & 0xF0) == 0xE0 && i + 2 < n) {
            i += 3;
        } else if ((c & 0xF8) == 0xF0 && i + 3 < n) {
            i += 4;
        } else {
            i++;
        }
    }
    while (tlen > 0 && tmp[tlen-1] == ' ') tlen--;
    tmp[tlen] = '\0';
    strcpy(s, tmp);
}

/* ============================================================
 * Raw extraction: KEY(value) -> sirovi tekst (bez čišćenja)
 * Podržava: ASCII, UTF-16BE (BOM ili bez), PDF hex <...>
 * ============================================================ */
static int akd_izvuci_polje_raw(const unsigned char *buf, size_t len,
                                 const char *marker,
                                 char *out, size_t outlen) {
    if (outlen == 0) return 0;
    out[0] = '\0';
    void *p = memmem_case(buf, len, marker, strlen(marker));
    if (!p) return 0;

    const unsigned char *q = (const unsigned char*)p + strlen(marker);
    const unsigned char *end = buf + len;
    while (q < end && *q != '(') q++;
    if (q >= end) return 0;
    q++;
    const unsigned char *val_start = q;

    const unsigned char *val_end = NULL;
    const unsigned char *scan = q;
    while (scan < end) {
        if (*scan == '\\' && scan + 1 < end) { scan += 2; continue; }
        if (*scan == ')') { val_end = scan; break; }
        scan++;
    }
    if (!val_end) return 0;

    size_t raw_len = val_end - val_start;
    if (raw_len == 0) return 0;

    size_t i = 0;

    /* PDF hex <...> */
    if (raw_len >= 2 && val_start[0] == '<' && val_end[-1] == '>') {
        const unsigned char *hex = val_start + 1;
        size_t hexlen = raw_len - 2;
        size_t j = 0;
        int utf16 = 0;
        if (hexlen >= 4 &&
            ((hex[0]=='F'||hex[0]=='f') && (hex[1]=='E'||hex[1]=='e') &&
             (hex[2]=='F'||hex[2]=='f') && (hex[3]=='F'||hex[3]=='f'))) {
            utf16 = 1; j = 4;
        }
        if (utf16) {
            while (j + 3 < hexlen && i < outlen - 4) {
                unsigned char h1=hex[j], h2=hex[j+1], h3=hex[j+2], h4=hex[j+3];
                if (!isxdigit(h1)||!isxdigit(h2)||!isxdigit(h3)||!isxdigit(h4)) break;
                int v1=isdigit(h1)?h1-'0':(h1|32)-'a'+10;
                int v2=isdigit(h2)?h2-'0':(h2|32)-'a'+10;
                int v3=isdigit(h3)?h3-'0':(h3|32)-'a'+10;
                int v4=isdigit(h4)?h4-'0':(h4|32)-'a'+10;
                unsigned int cp = (v1<<12)|(v2<<8)|(v3<<4)|v4;
                i += utf16be_to_utf8(cp, out+i, outlen-i);
                j += 4;
            }
        } else {
            while (j + 1 < hexlen && i < outlen - 1) {
                unsigned char h1=hex[j], h2=hex[j+1];
                if (!isxdigit(h1)||!isxdigit(h2)) break;
                int v1=isdigit(h1)?h1-'0':(h1|32)-'a'+10;
                int v2=isdigit(h2)?h2-'0':(h2|32)-'a'+10;
                out[i++] = (char)((v1<<4)|v2);
                j += 2;
            }
        }
        out[i] = '\0';
        return (int)i;
    }

    /* UTF-16BE s BOM */
    if (raw_len >= 2 && val_start[0] == 0xFE && val_start[1] == 0xFF) {
        const unsigned char *r = val_start + 2;
        while (r + 1 < val_end && i < outlen - 4) {
            unsigned int cp = ((unsigned int)r[0] << 8) | r[1];
            r += 2;
            if (cp == 0x5C && r < val_end) {
                unsigned int next = ((unsigned int)r[0] << 8) | r[1];
                if (next == 0x7200 || next == 0x0072) {
                    r += 2;
                    continue;
                }
            }
            if ((cp >= 0x4E00 && cp <= 0x9FFF) ||
                (cp >= 0x3040 && cp <= 0x30FF) ||
                (cp >= 0xAC00 && cp <= 0xD7AF)) {
                continue;
            }
            if (cp == 0x015C) cp = 0x010D;
            i += utf16be_to_utf8(cp, out+i, outlen-i);
        }
        out[i] = '\0';
        return (int)i;
    }

    /* ASCII s escape */
    const unsigned char *r = val_start;
    while (r < val_end && i < outlen - 1) {
        if (*r == '\\' && r + 1 < val_end) {
            r++;
            if (*r == '(' || *r == ')' || *r == '\\') out[i++] = *r++;
            else out[i++] = '\\';
        } else {
            out[i++] = *r++;
        }
    }
    out[i] = '\0';
    return (int)i;
}

/* ============================================================
 * Wrapper: bez čišćenja (za URL, datum, zaglavlje)
 * ============================================================ */
static int akd_izvuci_polje(const unsigned char *buf, size_t len,
                             const char *marker,
                             char *out, size_t outlen) {
    return akd_izvuci_polje_raw(buf, len, marker, out, outlen);
}

/* ============================================================
 * Wrapper: s čišćenjem (samo za Potpisnik)
 * ============================================================ */
static int akd_izvuci_polje_ime(const unsigned char *buf, size_t len,
                                 const char *marker,
                                 char *out, size_t outlen) {
    int r = akd_izvuci_polje_raw(buf, len, marker, out, outlen);
    if (r > 0) akd_ocisti(out);
    return r;
}

/* ============================================================
 * Glavna funkcija: izvuci sve AKD metapodatke
 * ============================================================ */
int akd_izvuci_meta(const unsigned char *pdf_buf, size_t pdf_len, AKD_Meta *out) {
    memset(out, 0, sizeof(*out));
    if (!memmem_case(pdf_buf, pdf_len, "#23A010", 7)) return 0;
    out->pronaden = 1;
    akd_izvuci_polje(pdf_buf, pdf_len, "#23A010Zaglavlje", out->zaglavlje, sizeof(out->zaglavlje));
    akd_izvuci_polje(pdf_buf, pdf_len, "#23A020BrojZapisa", out->broj_zapisa, sizeof(out->broj_zapisa));
    akd_izvuci_polje(pdf_buf, pdf_len, "#23A030VrstaPotpisa", out->vrsta_potpisa, sizeof(out->vrsta_potpisa));
    akd_izvuci_polje(pdf_buf, pdf_len, "#23A040DatumVrijemePotpisa", out->datum_potpisa, sizeof(out->datum_potpisa));
    akd_izvuci_polje(pdf_buf, pdf_len, "#23A050IzdavateljCertifikata", out->izdavatelj, sizeof(out->izdavatelj));
    akd_izvuci_polje_ime(pdf_buf, pdf_len, "#23A060Potpisnik", out->potpisnik, sizeof(out->potpisnik));
    akd_izvuci_polje(pdf_buf, pdf_len, "#23A070PotpisnikPIN", out->potpisnik_pin, sizeof(out->potpisnik_pin));
    akd_izvuci_polje(pdf_buf, pdf_len, "#23A110Provjera", out->provjera_url, sizeof(out->provjera_url));
    akd_izvuci_polje(pdf_buf, pdf_len, "#23A990Podnozje", out->podnozje, sizeof(out->podnozje));
    if (out->podnozje[0] == '\0')
        akd_izvuci_polje(pdf_buf, pdf_len, "#23A150Podnozje", out->podnozje, sizeof(out->podnozje));
    if (out->zaglavlje[0] == '\0') {
        void *akd = memmem_case(pdf_buf, pdf_len, "AKD ePotpis", 11);
        if (akd) snprintf(out->zaglavlje, sizeof(out->zaglavlje), "AKD ePotpis");
    }
    if (out->potpisnik_pin[0] == '\0' && out->potpisnik[0] == '\0' &&
        out->datum_potpisa[0] == '\0') {
        memset(out, 0, sizeof(*out));
        return 0;
    }
    return 1;
}

/* ============================================================
 * Formatiraj
 * ============================================================ */
void akd_formatiraj(const AKD_Meta *m, char *out, size_t outlen) {
    if (!m->pronaden) { snprintf(out, outlen, "Nije prisutan"); return; }
    char buf[1024]; int n = 0;
    if (m->potpisnik[0]) n += snprintf(buf+n, sizeof(buf)-n, "Potpisnik=%s", m->potpisnik);
    if (m->potpisnik_pin[0]) n += snprintf(buf+n, sizeof(buf)-n, "%sPIN=%s", n?" | ":"", m->potpisnik_pin);
    if (m->datum_potpisa[0]) n += snprintf(buf+n, sizeof(buf)-n, "%sDatum=%s", n?" | ":"", m->datum_potpisa);
    if (m->vrsta_potpisa[0]) n += snprintf(buf+n, sizeof(buf)-n, "%sVrsta=%s", n?" | ":"", m->vrsta_potpisa);
    if (m->zaglavlje[0]) n += snprintf(buf+n, sizeof(buf)-n, "%sAlat=%s", n?" | ":"", m->zaglavlje);
    if (m->provjera_url[0]) n += snprintf(buf+n, sizeof(buf)-n, "%sProvjera=%s", n?" | ":"", m->provjera_url);
    snprintf(out, outlen, "%s", n ? buf : "AKD widget bez prepoznatih polja");
}
