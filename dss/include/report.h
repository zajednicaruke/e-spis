#ifndef REPORT_H
#define REPORT_H

#include <stddef.h>

void report_init(void);
void report_dodaj(const char *fmt, ...);
const char *report_buf(void);
size_t report_len(void);
void report_dodaj_dokument(const char *ime, const char *klasa, const char *urbroj,
                           const char *velicina, const char *modificirano, const char *sha256,
                           int qes_status, const char *qes_meta);
void report_dodaj_zbirni(const char *folder, const char *zbirni_sha256, int ukupno, long ukupno_bajtova);

#endif
