#ifndef UTIL_H
#define UTIL_H
#include <stddef.h>
#include <time.h>

/* Binarna pretraga bez obzira na velika/mala slova */
void *memmem_case(const void *haystack, size_t hlen,
                  const void *needle, size_t nlen);

/* Formatiraj vrijeme u "dd.mm.yyyy HH:MM:SS" */
void fmt_time(time_t t, char out[32]);

/* Vrati pokazivač na ime datoteke iz pune putanje */
const char *path_basename(const char *path);

/* Formatiraj veličinu u čitljiv format */
void fmt_size(long long bytes, char out[32]);

/* Izvuci string iz binarnog buffera, ignorirajući whitespace i separatore */
int extract_pdf_string(const unsigned char *buf, size_t buflen,
                       const char *marker,
                       char *out, size_t outlen);

#endif
