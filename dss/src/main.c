#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include "scan.h"
#include "report.h"
#include "pdf.h"

static void ukloni_novi_red(char *s) {
    size_t len = strlen(s);
    while (len > 0 && (s[len - 1] == '\n' || s[len - 1] == '\r' || s[len - 1] == ' ' || s[len - 1] == '\t')) {
        s[--len] = '\0';
    }
}

int main(int argc, char **argv) {
    char unos[1024] = {0};
    char dirpath[2048] = {0};
    char out_pdf[2300] = {0};

    if (argc > 1 && argv[1][0] != '\0') {
        snprintf(unos, sizeof(unos), "%s", argv[1]);
    } else {
        if (isatty(STDIN_FILENO)) {
            printf("Unesite naziv mape na /sdcard (npr. 01 HEP): ");
            fflush(stdout);
        }
        if (!fgets(unos, sizeof(unos), stdin)) {
            fprintf(stderr, "[XX] Nije unesen naziv mape.\n");
            return 1;
        }
    }

    ukloni_novi_red(unos);
    if (unos[0] == '\0') {
        fprintf(stderr, "[XX] Prazan unos mape.\n");
        return 1;
    }

    if (unos[0] == '/') {
        snprintf(dirpath, sizeof(dirpath), "%s", unos);
    } else {
        snprintf(dirpath, sizeof(dirpath), "/sdcard/%s", unos);
    }

    struct stat st;
    if (stat(dirpath, &st) != 0 || !S_ISDIR(st.st_mode)) {
        fprintf(stderr, "[XX] Mapa ne postoji ili nije dostupna: %s\n", dirpath);
        return 1;
    }

    printf("========================================================================\n");
    printf("  Zajednica RUKE — DSSvalidator v5.0 (C23 Sovereign Engine)             \n");
    printf("  Skeniranje mape: %s\n", dirpath);
    printf("========================================================================\n");

    report_init();

    Scan_Rezultat rez;
    memset(&rez, 0, sizeof(rez));
    scan_direktorij(dirpath, &rez);

    report_dodaj_zbirni(dirpath, rez.zbirni_hash, rez.broj_dokumenata, (long)rez.ukupna_kolicina);

    snprintf(out_pdf, sizeof(out_pdf), "%s/Nalaz_Validacije_Zajednica_RUKE.pdf", dirpath);
    pdf_generiraj(out_pdf, report_buf(), report_len());

    printf("[OK] Obradeno akata: %d (%lld bajtova)\n", rez.broj_dokumenata, rez.ukupna_kolicina);
    printf("[OK] Zbirni SHA-256: %s\n", rez.zbirni_hash);
    printf("[OK] Generiran nalaz: %s\n", out_pdf);
    printf("========================================================================\n");

    return 0;
}
