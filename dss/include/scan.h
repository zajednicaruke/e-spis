#ifndef SCAN_H
#define SCAN_H
#include <stddef.h>

typedef struct {
    char folder[2048];
    char zbirni_hash[65];
    int  broj_dokumenata;
    long long ukupna_kolicina;
} Scan_Rezultat;

void scan_direktorij(const char *dirpath, Scan_Rezultat *rez);

int scan_je_vlastiti_nalaz(const char *filename);

#endif
