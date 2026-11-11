#include "report.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

static char g_report_buf[524288];
static size_t g_report_len = 0;

void report_init(void) {
    g_report_buf[0] = '\0';
    g_report_len = 0;
}

void report_reset(void) {
    report_init();
}

const char *report_buf(void) {
    return g_report_buf;
}

size_t report_len(void) {
    return g_report_len;
}

void report_dodaj(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    char temp_buf[2048];
    int n = vsnprintf(temp_buf, sizeof(temp_buf), fmt, args);
    va_end(args);
    if (n > 0 && g_report_len + (size_t)n < sizeof(g_report_buf) - 1) {
        memcpy(g_report_buf + g_report_len, temp_buf, (size_t)n);
        g_report_len += (size_t)n;
        g_report_buf[g_report_len] = '\0';
    }
}

const char *report_dohvati_tekst(size_t *out_len) {
    if (out_len) *out_len = g_report_len;
    return g_report_buf;
}

void report_dodaj_dokument(const char *ime, const char *klasa, const char *urbroj,
                           const char *velicina, const char *modificirano, const char *sha256,
                           int qes_status, const char *qes_meta) {
    report_dodaj("DOKUMENT: %s\n", ime);
    report_dodaj("KLASA: %s\n", klasa);
    report_dodaj("URBROJ: %s\n", urbroj);
    report_dodaj("VELICINA: %s\n", velicina);
    report_dodaj("MODIFICIRANO: %s\n", modificirano);
    report_dodaj("SHA-256: %s\n", sha256);
    if (qes_status == 1) {
        report_dodaj("QES: DA (Potpisan)\n");
        if (qes_meta && qes_meta[0]) {
            report_dodaj("QES METAPODACI:\n%s\n", qes_meta);
        }
    } else {
        report_dodaj("QES: NE (Nije potpisan)\n");
    }
    report_dodaj("STATUS: ZAPRIMLJENO I VALIDIRANO\n");
    report_dodaj("----------------------------------------------------------------\n\n");
}

void report_dodaj_zbirni(const char *folder, const char *zbirni_sha256, int ukupno, long ukupno_bajtova) {
    report_dodaj("================================================================\n");
    report_dodaj("ZBIRNI IZVJEŠTAJ KOLEKCIJE: %s\n", folder);
    report_dodaj("Ukupno analizirano akata: %d\n", ukupno);
    report_dodaj("Ukupna veličina: %ld bajtova\n", ukupno_bajtova);
    report_dodaj("Merkle/Zbirni SHA-256 otisak: %s\n", zbirni_sha256);
    report_dodaj("================================================================\n");
}
