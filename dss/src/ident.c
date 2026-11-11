#include "ident.h"
#include <stdio.h>
#include <ctype.h>
#include <string.h>

void ident_generiraj(const char *hex_hash,
                     char *klasa, size_t klasa_len,
                     char *urbroj, size_t urbroj_len) {
    unsigned int d1, d2, d3, d4, d5;
    sscanf(hex_hash, "%4x%2x%2x%2x%2x", &d1, &d2, &d3, &d4, &d5);
    snprintf(klasa, klasa_len,
             "UP/I-%03d-%02d/26-%02d/%02d",
             (d1 % 900) + 100, (d2 % 9) + 1,
             (d3 % 99) + 1,    (d4 % 99) + 1);
    snprintf(urbroj, urbroj_len,
             "6451-26-%02d-%02d-%d",
             (d5 % 9) + 1, (d2 % 99) + 1, (d1 % 9) + 1);
}

void ident_kontrolni_kod(const char *hex_hash, char out[10]) {
    snprintf(out, 10, "%.4s-%.4s", hex_hash, hex_hash + 4);
    for (int i = 0; out[i]; i++)
        if (out[i] >= 'a' && out[i] <= 'z') out[i] -= 32;
}
