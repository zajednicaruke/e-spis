#ifndef IDENT_H
#define IDENT_H
#include <stddef.h>

/* Generiraj klasu i urbroj iz SHA-256 hasha.
   Deterministički: isti hash -> ista klasa i urbroj. */
void ident_generiraj(const char *hex_hash,
                     char *klasa, size_t klasa_len,
                     char *urbroj, size_t urbroj_len);

/* Vrati kontrolni kod "XXXX-XXXX" iz hex hasha */
void ident_kontrolni_kod(const char *hex_hash, char out[10]);

#endif
