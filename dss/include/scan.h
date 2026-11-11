#ifndef SCAN_H
#define SCAN_H
#include <stddef.h>

typedef struct {
    char folder[2048];
    char zbirni_hash[65];
    int  broj_dokumenata;
    long long ukupna_kolicina;
} Scan_Rezultat;

/* Skeniraj jedan direktorij i sve regularne datoteke u njemu.
   Rezultat se puni (zbirni_hash, broj_dokumenata, ukupna_kolicina). */
void scan_direktorij(const char *dirpath, Scan_Rezultat *rez);

/* Provjeri je li datoteka vlastiti PDF nalaz (da se ne rekurzira) */
int scan_je_vlastiti_nalaz(const char *filename);

#endif
