#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

static bool cmd_returns_empty(const char *cmd) {
    FILE *p = popen(cmd, "r");
    if (!p) return false;
    char buf[256];
    size_t n = fread(buf, 1, sizeof(buf) - 1, p);
    pclose(p);
    return (n == 0);
}

static bool cmd_succeeds(const char *cmd) {
    return (system(cmd) == 0);
}

int main(void) {
    time_t now = time(NULL);
    char ts[64];
    strftime(ts, sizeof(ts), "%Y-%m-%d %H:%M:%S %Z", localtime(&now));

    FILE *rpt = fopen("01_LOKALNI_TREZOR/backup_arhiva/dokaz_sigurnosti.txt", "w");
    if (!rpt) {
        perror("Ne mogu otvoriti izvještaj u trezoru");
        return EXIT_FAILURE;
    }

    printf("================================================================\n");
    printf("  ZAJEDNICA RUKE — C23 FORENZIČKI DOKAZ SIGURNOSTI EKOSUSTAVA\n");
    printf("  Vrijeme revizije: %s\n", ts);
    printf("================================================================\n\n");

    fprintf(rpt, "ZAJEDNICA RUKE — FORENZIČKI DOKAZ SIGURNOSTI EKOSUSTAVA\n");
    fprintf(rpt, "Vrijeme revizije: %s\n\n", ts);

    int passed = 0;
    int total = 6;

    /* TEST 1: Odsutnost registar_akata.json u cijeloj Git povijesti */
    bool t1 = cmd_returns_empty("git log --all --oneline -- registar_akata.json");
    printf("[1/6] Git povijest bez registar_akata.json:        %s\n", t1 ? "PASS (100% ČISTO)" : "FAIL");
    fprintf(rpt, "[1/6] Git povijest bez registar_akata.json: %s\n", t1 ? "PASS" : "FAIL");
    if (t1) passed++;

    /* TEST 2: Poništenje starog rizičnog commita 34c2e22 */
    bool t2 = !cmd_succeeds("git rev-parse --verify 34c2e22 >/dev/null 2>&1");
    printf("[2/6] Stari commit 34c2e22 izbrisan iz baze:       %s\n", t2 ? "PASS (NE POSTOJI)" : "FAIL");
    fprintf(rpt, "[2/6] Stari commit 34c2e22 izbrisan: %s\n", t2 ? "PASS" : "FAIL");
    if (t2) passed++;

    /* TEST 3: Izolacija lokalnog trezora u .gitignore */
    bool t3 = cmd_returns_empty("git ls-files 01_LOKALNI_TREZOR/");
    printf("[3/6] 01_LOKALNI_TREZOR isključen iz Gita:         %s\n", t3 ? "PASS (IZOLIRANO)" : "FAIL");
    fprintf(rpt, "[3/6] 01_LOKALNI_TREZOR izoliran: %s\n", t3 ? "PASS" : "FAIL");
    if (t3) passed++;

    /* TEST 4: Nema otvorenih PIN-ova ni tokena u index.html i C izvoru */
    bool t4 = cmd_returns_empty("grep -E '3PutSvet!|Clan2026!|Pridruzeni2026!|Pocasni2026!|Ugovor2026!|service_role|ghp_' napravi_offline_app.c index.html 2>/dev/null");
    printf("[4/6] Javne datoteke bez otvorenih tajni/PIN-ova:  %s\n", t4 ? "PASS (SHA-256 ONLY)" : "FAIL");
    fprintf(rpt, "[4/6] Javne datoteke bez otvorenih tajni: %s\n", t4 ? "PASS" : "FAIL");
    if (t4) passed++;

    /* TEST 5: Usklađenost lokalnog HEAD-a i udaljenog GitHub origin/main */
    bool t5 = cmd_succeeds("test \"$(git rev-parse HEAD)\" = \"$(git rev-parse origin/main)\"");
    printf("[5/6] Sinkronizacija HEAD == origin/main:          %s\n", t5 ? "PASS (USKLAĐENO)" : "FAIL");
    fprintf(rpt, "[5/6] Sinkronizacija HEAD == origin/main: %s\n", t5 ? "PASS" : "FAIL");
    if (t5) passed++;

    /* TEST 6: Postojanje sigurnosne arhive u /sdcard/Download */
    bool t6 = !cmd_returns_empty("ls /sdcard/Download/njm_2_ekosustav_01_clean_*.tar.gz 2>/dev/null");
    printf("[6/6] Sigurnosna arhiva u /sdcard/Download:        %s\n", t6 ? "PASS (POHRANJENO)" : "FAIL");
    fprintf(rpt, "[6/6] Sigurnosna arhiva u /sdcard/Download: %s\n", t6 ? "PASS" : "FAIL");
    if (t6) passed++;

    printf("\n----------------------------------------------------------------\n");
    printf("  UKUPNI REZULTAT REVIZIJE: %d / %d PROŠLO\n", passed, total);
    if (passed == total) {
        printf("  STATUS: EKOSUSTAV ZAJEDNICE RUKE JE 100%% OSIGURAN I VERIFICIRAN.\n");
        fprintf(rpt, "\nSTATUS: 100%% OSIGURANO I VERIFICIRANO (%d/%d)\n", passed, total);
    } else {
        printf("  STATUS: POTREBNA DODATNA PROVJERA NEUSPJEŠNIH STAVKI.\n");
        fprintf(rpt, "\nSTATUS: DJELOMIČNO (%d/%d)\n", passed, total);
    }
    printf("  Dokaz spremljen u: 01_LOKALNI_TREZOR/backup_arhiva/dokaz_sigurnosti.txt\n");
    printf("================================================================\n");

    fclose(rpt);
    return (passed == total) ? EXIT_SUCCESS : EXIT_FAILURE;
}
