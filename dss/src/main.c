#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "scan.h"
#include "util.h"
#include "report.h"
#include "pdf.h"

int main(void) {
    printf("=================================================================\n");
    printf("  ZASLONSKI SUSTAV e-SPISA — /DSSvalidator v4.9\n");
    printf("  Institut Zajednice RUKE (OIB: 64517735227)\n");
    printf("=================================================================\n");

    char unos[256];
    printf("Unesite naziv glavnog foldera (npr. 01 HEP):\n> ");
    fflush(stdout);

    if (!fgets(unos, sizeof(unos), stdin)) {
        return EXIT_FAILURE;
    }
    unos[strcspn(unos, "\r\n")] = 0;

    if (unos[0] == '\0') {
        printf("Prazan unos.\n");
        return EXIT_FAILURE;
    }

    report_init();

    char zbirni_sha256[65] = {0};
    long long ukupna_kolicina = 0;
    int ukupno = 0;
    char zadnji[512] = {0};
    int pronadjeno = 0;

    const char *baze[] = {"/sdcard", "/sdcard/Download", NULL};

    for (int i = 0; baze[i]; i++) {
        char putanja[512];
        snprintf(putanja, sizeof(putanja), "%s/%s", baze[i], unos);

        Scan_Rezultat rez;
        scan_direktorij(putanja, &rez);

        if (rez.broj_dokumenata > 0) {
            pronadjeno = 1;
            ukupno += rez.broj_dokumenata;
            ukupna_kolicina += rez.ukupna_kolicina;
            strncpy(zbirni_sha256, rez.zbirni_hash, sizeof(zbirni_sha256) - 1);

            // Ispravno prosljeđivanje numeričke veličine u bajtovima (long)
            report_dodaj_zbirni(putanja, rez.zbirni_hash, rez.broj_dokumenata, (long)rez.ukupna_kolicina);

            snprintf(zadnji, sizeof(zadnji), "%s/Nalaz_Validacije_Zajednica_RUKE.pdf", putanja);
            
            // Eksplicitno generiranje PDF-a odmah po obradi foldera
            pdf_generiraj(zadnji, report_buf(), report_len());
            break;
        }
    }

    if (!pronadjeno) {
        printf("\nNijedan folder u /sdcard ne sadrzi: '%s'\n", unos);
        return EXIT_FAILURE;
    }

    printf("-----------------------------------------------------------------\n");
    if (zadnji[0]) {
        printf("📄 PDF NALAZ GENERIRAN: %s\n", zadnji);
    }
    printf("Ukupno validirano dokumenata:  %d\n", ukupno);
    char uk_vel[32];
    fmt_size(ukupna_kolicina, uk_vel);
    printf("Ukupna velicina kolekcije:     %s\n", uk_vel);
    printf("=================================================================\n");

    return EXIT_SUCCESS;
}
