#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/stat.h>

static bool dir_exists(const char *path) {
    struct stat st;
    return (stat(path, &st) == 0 && S_ISDIR(st.st_mode));
}

static bool scan_file_for_leaks(const char *filepath) {
    FILE *f = fopen(filepath, "rb");
    if (!f) return true; // Ako datoteka ne postoji, nema curenja

    fseek(f, 0, SEEK_END);
    long len = ftell(f);
    if (len <= 0) { fclose(f); return true; }
    rewind(f);

    char *buf = malloc((size_t)len + 1);
    if (!buf) { fclose(f); return false; }
    size_t r = fread(buf, 1, (size_t)len, f);
    buf[r] = '\0';
    fclose(f);

    const char *forbidden[] = {
        "service_role",
        "ghp_",
        "oib_potpisnika",
        "3PutSvet!",
        "Clan2026!",
        "RUKE2026",
        "Pridruzeni2026!",
        "Pocasni2026!",
        "Ugovor2026!"
    };
    size_t n = sizeof(forbidden) / sizeof(forbidden[0]);
    bool clean = true;

    for (size_t i = 0; i < n; i++) {
        if (strstr(buf, forbidden[i]) != NULL) {
            fprintf(stderr, "[KRITIČNO UPOZORENJE] Datoteka '%s' sadrži zabranjeni niz: %s\n", filepath, forbidden[i]);
            clean = false;
        }
    }

    free(buf);
    return clean;
}

int main(void) {
    printf("=== C23 SIGURNOSNI PROTOKOL: PROVJERA SLOJEVA ===\n");

    const char *layers[] = {
        "01_LOKALNI_TREZOR/tajne",
        "01_LOKALNI_TREZOR/izvorni_akti",
        "01_LOKALNI_TREZOR/backup_arhiva",
        "02_GITHUB_JAVNO",
        "03_SUPABASE_MOST/qes_za_upload",
        "03_SUPABASE_MOST/javni_metapodaci"
    };

    for (size_t i = 0; i < 6; i++) {
        if (dir_exists(layers[i])) {
            printf("[OK] Sloj aktivan: %s\n", layers[i]);
        } else {
            fprintf(stderr, "[GREŠKA] Nedostaje sloj: %s\n", layers[i]);
            return EXIT_FAILURE;
        }
    }

    printf("\n=== SKENIRANJE JAVNIH DATOTEKA PRIJE GIT PUSH-a ===\n");
    bool safe_c = scan_file_for_leaks("napravi_offline_app.c");
    bool safe_html = scan_file_for_leaks("index.html");
    bool safe_readme = scan_file_for_leaks("README.md");

    if (!safe_c || !safe_html || !safe_readme) {
        fprintf(stderr, "\n[BLOKADA] Otkriveno curenje tajni ili starih lozinki! Git push je zabranjen dok se ne očiste.\n");
        return EXIT_FAILURE;
    }

    printf("\n[ZELENO SVJETLO] Svi javni slojevi su čisti. Sigurno za slanje na GitHub.\n");
    return EXIT_SUCCESS;
}
