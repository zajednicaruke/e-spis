#ifndef REPORT_H
#define REPORT_H
#include <stddef.h>

#define MAX_REPORT_SIZE (1024 * 1024)

/* Inicijaliziraj report buffer s naslovom */
void report_init(const char *datum);

/* Dodaj formatirani tekst u buffer */
void report_dodaj(const char *format, ...);

/* Dodaj jedan zapis o dokumentu */
void report_dodaj_dokument(const char *ime,
                            const char *klasa,
                            const char *urbroj,
                            const char *velicina,
                            const char *modificirano,
                            const char *sha256,
                            int qes_potpisan,
                            const char *qes_metapodaci);

/* Dodaj zbirni hash + statistiku */
void report_dodaj_zbirni(const char *folder,
                          const char *zbirni_hash,
                          int broj_dokumenata,
                          const char *ukupna_velicina);

/* Pristup bufferu (za PDF generator) */
const char *report_buf(void);
size_t      report_len(void);

#endif
