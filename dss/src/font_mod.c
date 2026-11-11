#include "font_mod.h"
#include <string.h>
#include <stdio.h>
#include <stdint.h>

static unsigned int utf8_decode(const unsigned char *s, size_t *consumed) {
    if (!s[0]) { *consumed = 0; return 0; }
    unsigned char c = s[0];
    if (c < 0x80) { *consumed = 1; return c; }
    if ((c & 0xE0) == 0xC0 && s[1]) { *consumed = 2; return ((c & 0x1F) << 6) | (s[1] & 0x3F); }
    if ((c & 0xF0) == 0xE0 && s[1] && s[2]) { *consumed = 3; return ((c & 0x0F) << 12) | ((s[1] & 0x3F) << 6) | (s[2] & 0x3F); }
    if ((c & 0xF8) == 0xF0 && s[1] && s[2] && s[3]) { *consumed = 4; return ((c & 0x07) << 18) | ((s[1] & 0x3F) << 12) | ((s[2] & 0x3F) << 6) | (s[3] & 0x3F); }
    *consumed = 1;
    return '?';
}

int font_mod_utf8_to_utf16be_hex(const char *input, char *output, size_t max_out) {
    if (!input || !output || max_out < 8) return 0;
    size_t j = 0;
    for (size_t i = 0; input[i] != '\0' && j < max_out - 8; ) {
        size_t consumed = 1;
        unsigned int cp = utf8_decode((const unsigned char*)&input[i], &consumed);
        if (consumed == 0) break;
        i += consumed;

        if (cp < 0x20 && cp != ' ') continue;

        if (cp < 0x10000) {
            j += snprintf(output + j, max_out - j, "%04X", cp);
        } else {
            cp -= 0x10000;
            unsigned int hi = 0xD800 + (cp >> 10);
            unsigned int lo = 0xDC00 + (cp & 0x3FF);
            j += snprintf(output + j, max_out - j, "%04X%04X", hi, lo);
        }
    }
    return (int)j;
}

size_t font_mod_transliterate(const char *input, char *output, size_t max_out) {
    if (!input || !output || max_out == 0) return 0;
    size_t j = 0;
    for (size_t i = 0; input[i] != '\0' && j < max_out - 4; ) {
        unsigned char c = (unsigned char)input[i];
        if (c == '(' || c == ')' || c == '\\') {
            output[j++] = '\\';
            output[j++] = (char)c;
            i++;
        } else if (c < 128) {
            if (c >= 32) output[j++] = (char)c;
            else output[j++] = ' ';
            i++;
        } else {
            // Propuštamo izvorne UTF-8 bajtove za hrvatske dijakritike netaknute
            output[j++] = (char)c;
            i++;
        }
    }
    output[j] = '\0';
    return j;
}
