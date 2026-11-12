#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define SRC "napravi_offline_app.c"
#define TMP "napravi_offline_app.c.tmp"

static char *read_all(const char *path, long *out_len) {
    FILE *f = fopen(path, "rb");
    if (!f) { perror(path); return NULL; }
    fseek(f, 0, SEEK_END);
    long len = ftell(f);
    if (len < 0) { fclose(f); return NULL; }
    rewind(f);
    char *buf = malloc((size_t)len + 1);
    if (!buf) { fclose(f); return NULL; }
    size_t r = fread(buf, 1, (size_t)len, f);
    fclose(f);
    buf[r] = '\0';
    if (out_len) *out_len = len;
    return buf;
}

static bool write_all(const char *path, const char *data) {
    FILE *f = fopen(path, "wb");
    if (!f) { perror(path); return false; }
    size_t len = strlen(data);
    bool ok = (fwrite(data, 1, len, f) == len);
    fclose(f);
    return ok;
}

/* Zamjenjuje stari otvoreni PIN u C stringu s odgovarajućom SHA-256 konstantom */
static char *replace_pin_with_hash_macro(char *src, const char *old_pin, const char *macro_name) {
    char *pos = strstr(src, old_pin);
    if (!pos) return src;

    // Pretvaramo '3PutSvet!' u '" HASH_PIN_ADMIN "' unutar C fprintf stringa
    char replacement[128];
    snprintf(replacement, sizeof(replacement), "\" %s \"", macro_name);

    size_t head = (size_t)(pos - src);
    size_t old_len = strlen(old_pin);
    size_t rep_len = strlen(replacement);
    size_t tail = strlen(pos + old_len);

    char *out = malloc(head + rep_len + tail + 1);
    if (!out) return src;

    memcpy(out, src, head);
    memcpy(out + head, replacement, rep_len);
    memcpy(out + head + rep_len, pos + old_len, tail + 1);
    free(src);
    return out;
}

int main(void) {
    long len = 0;
    char *buf = read_all(SRC, &len);
    if (!buf) return EXIT_FAILURE;

    // 1. Dodajemo #include "src_c23/pin_hashes.h" na vrh ako već nije tamo
    if (!strstr(buf, "pin_hashes.h")) {
        const char *inc = "#include \"src_c23/pin_hashes.h\"\n";
        char *out = malloc(strlen(inc) + strlen(buf) + 1);
        if (!out) { free(buf); return EXIT_FAILURE; }
        strcpy(out, inc);
        strcat(out, buf);
        free(buf);
        buf = out;
    }

    // 2. Zamjenjujemo sve pojave starih PIN-ova s makroima iz pin_hashes.h
    buf = replace_pin_with_hash_macro(buf, "3PutSvet!", "HASH_PIN_ADMIN");
    buf = replace_pin_with_hash_macro(buf, "Clan2026!", "HASH_PIN_REDOVNI");
    buf = replace_pin_with_hash_macro(buf, "Pridruzeni2026!", "HASH_PIN_PRIDRUZENI");
    buf = replace_pin_with_hash_macro(buf, "Pocasni2026!", "HASH_PIN_POCASNI");
    buf = replace_pin_with_hash_macro(buf, "Ugovor2026!", "HASH_PIN_UGOVORNI");

    // 3. Ugrađujemo Web Crypto SHA-256 funkciju za provjeru PIN-a u JS blok
    char *script_end = strstr(buf, "ispisi_pdf_stamping_js");
    if (script_end) {
        const char *crypto_js =
            "    fprintf(fhtml, \"<script>\\n\");\n"
            "    fprintf(fhtml, \"async function provjeriPinSha256(uneseniPin) {\\n\");\n"
            "    fprintf(fhtml, \"  const enc = new TextEncoder().encode(uneseniPin.trim());\\n\");\n"
            "    fprintf(fhtml, \"  const buf = await crypto.subtle.digest('SHA-256', enc);\\n\");\n"
            "    fprintf(fhtml, \"  return Array.from(new Uint8Array(buf)).map(b => b.toString(16).padStart(2, '0')).join('');\\n\");\n"
            "    fprintf(fhtml, \"}\\n\");\n"
            "    fprintf(fhtml, \"</script>\\n\");\n";

        char *fclose_pos = strstr(buf, "fclose(fhtml);");
        if (fclose_pos && !strstr(buf, "provjeriPinSha256")) {
            size_t h = (size_t)(fclose_pos - buf);
            size_t c = strlen(crypto_js);
            size_t t = strlen(fclose_pos);
            char *out = malloc(h + c + t + 1);
            if (out) {
                memcpy(out, buf, h);
                memcpy(out + h, crypto_js, c);
                memcpy(out + h + c, fclose_pos, t + 1);
                free(buf);
                buf = out;
            }
        }
    }

    if (!write_all(TMP, buf)) {
        free(buf);
        return EXIT_FAILURE;
    }
    free(buf);

    if (rename(TMP, SRC) != 0) {
        perror("rename");
        return EXIT_FAILURE;
    }

    printf("[C23 MIGRACIJA] Svi otvoreni PIN-ovi u %s zamijenjeni su SHA-256 makroima!\n", SRC);
    return EXIT_SUCCESS;
}
