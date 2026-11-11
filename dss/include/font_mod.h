#ifndef FONT_MOD_H
#define FONT_MOD_H
#include <stddef.h>

/* UTF-8 → UTF-16BE hex (za Identity-H s TTF fontom) */
int font_mod_utf8_to_utf16be_hex(const char *input, char *output, size_t max_out);

/* Transliteracija (sigurni fallback za Type1 / Helvetica) */
size_t font_mod_transliterate(const char *input, char *output, size_t max_out);

#endif
