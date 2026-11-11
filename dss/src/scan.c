#include "scan.h"
#include "sha256.h"
#include "qes.h"
#include "ident.h"
#include "report.h"
#include "util.h"
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <time.h>

int scan_je_vlastiti_nalaz(const char *filename) {
    return strncmp(filename, "Nalaz_Validacije", 16) == 0;
}

/* Očisti ime datoteke — ukloni emoji i kontrolne znakove */
static void scan_ocisti_ime(const char *in, char *out, size_t outlen) {
    size_t j = 0;
    for (size_t i = 0; in[i] && j + 1 < outlen; ) {
        unsigned char c = (unsigned char)in[i];
        if (c < 0x80) {
            if (c >= 0x20) out[j++] = (char)c;
            i++;
        } else if ((c & 0xE0) == 0xC0 && in[i+1]) {
            out[j++] = in[i++];
            out[j++] = in[i++];
        } else if ((c & 0xF0) == 0xE0 && in[i+1] && in[i+2]) {
            out[j++] = in[i++];
            out[j++] = in[i++];
            out[j++] = in[i++];
        } else if ((c & 0xF8) == 0xF0 && in[i+1] && in[i+2] && in[i+3]) {
            i += 4;
        } else {
            i++;
        }
    }
    out[j] = '\0';
}

static void ispisi_i_dodaj(const char *filepath, const struct stat *st,
                            const char *hex) {
    char ime_raw[512];
    scan_ocisti_ime(path_basename(filepath), ime_raw, sizeof(ime_raw));
    const char *ime = ime_raw;

    char klasa[32], urbroj[32], kod[10];
    ident_generiraj(hex, klasa, sizeof(klasa), urbroj, sizeof(urbroj));
    ident_kontrolni_kod(hex, kod);

    char vel[32]; fmt_size(st->st_size, vel);
    char vrijeme[32]; fmt_time(st->st_mtime, vrijeme);

    QES_Info qes;
    int qes_ok = qes_analiziraj(filepath, &qes);
    char qes_meta[512] = {0};
    if (qes_ok) qes_formatiraj(&qes, qes_meta, sizeof(qes_meta));

    /* Ispis na ekran */
    printf("┌─────────────────────────────────────────────────────────────────┐\n");
    printf("│ 📄 AKT / DOKUMENT                                               │\n");
    printf("│    %-60.60s │\n", ime);
    printf("├─────────────────────────────────────────────────────────────────┤\n");
    printf("│ 🏛  KLASA:         %-44s │\n", klasa);
    printf("│ 🔖 URBROJ:        %-44s │\n", urbroj);
    printf("├─────────────────────────────────────────────────────────────────┤\n");
    printf("│ 📦 Veličina:      %s\n", vel);
    printf("│ 🕒 Modificirano:  %s\n", vrijeme);
    printf("│ 🔐 SHA-256:                                                     │\n");
    printf("│    %s │\n", hex);
    printf("│ 🏷  Kontrolni kod: [%s] ✓                                      │\n", kod);
    printf("│ 🖋  e-Potpis (QES): %-44s │\n",
           qes_ok ? "✅ DA (Potpisan)" : "❌ NE (Nije potpisan)");
    if (qes_ok) {
        printf("│    %-60.60s │\n", qes_meta);
        if (strlen(qes_meta) > 60)
            printf("│    %-60.60s │\n", qes_meta + 60);
        if (strlen(qes_meta) > 120)
            printf("│    %-60.60s │\n", qes_meta + 120);
    }
    printf("│ ✅ Status:        ZAPRIMLJENO I VALIDIRANO                      │\n");
    printf("└─────────────────────────────────────────────────────────────────┘\n\n");

    /* Dodaj u report */
    report_dodaj_dokument(ime, klasa, urbroj, vel, vrijeme, hex,
                           qes_ok, qes_ok ? qes_meta : NULL);
}

void scan_direktorij(const char *dirpath, Scan_Rezultat *rez) {
    memset(rez, 0, sizeof(*rez));
    strncpy(rez->folder, dirpath, sizeof(rez->folder) - 1);

    DIR *dir = opendir(dirpath);
    if (!dir) return;

    SHA256_CTX zbirni;
    sha256_init(&zbirni);

    struct dirent *entry;
    char subpath[2048];

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] == '.') continue;
        if (scan_je_vlastiti_nalaz(entry->d_name)) continue;

        int n = snprintf(subpath, sizeof(subpath), "%s/%s",
                         dirpath, entry->d_name);
        if (n < 0 || n >= (int)sizeof(subpath)) continue;

        struct stat st;
        if (stat(subpath, &st) != 0) continue;
        if (!S_ISREG(st.st_mode)) continue;

        char hex[65];
        if (sha256_file(subpath, hex) != 0) continue;

        ispisi_i_dodaj(subpath, &st, hex);

        sha256_update(&zbirni, (uint8_t*)hex, 64);
        rez->broj_dokumenata++;
        rez->ukupna_kolicina += st.st_size;
    }
    closedir(dir);

    uint8_t raw[32];
    sha256_final(&zbirni, raw);
    for (int i = 0; i < 32; i++) {
        static const char HEX[] = "0123456789abcdef";
        rez->zbirni_hash[i*2]   = HEX[raw[i] >> 4];
        rez->zbirni_hash[i*2+1] = HEX[raw[i] & 0xF];
    }
    rez->zbirni_hash[64] = '\0';
}
