#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>

static int exec_step(const char *label, const char *cmd) {
    printf("\n[C23 KORAK] %s...\n", label);
    fflush(stdout);
    int rc = system(cmd);
    if (rc != 0) {
        fprintf(stderr, "[C23 GREŠKA] Korak neuspješan (izlazni kod %d): %s\n", rc, cmd);
        return -1;
    }
    printf("[C23 OK] %s završeno.\n", label);
    return 0;
}

int main(void) {
    const char *home = getenv("HOME");
    if (!home) {
        fprintf(stderr, "[C23 GREŠKA] HOME varijabla nije postavljena.\n");
        return EXIT_FAILURE;
    }

    char repo_dir[512];
    snprintf(repo_dir, sizeof(repo_dir), "%s/njm_2_ekosustav_01", home);
    if (chdir(repo_dir) != 0) {
        perror("[C23 GREŠKA] Ne mogu ući u direktorij repozitorija");
        return EXIT_FAILURE;
    }

    /* 1. Generiranje vremenske oznake za sigurnosni backup */
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    char timestamp[32];
    strftime(timestamp, sizeof(timestamp), "%Y%m%d_%H%M%S", tm_info);

    char backup_cmd[1024];
    snprintf(backup_cmd, sizeof(backup_cmd),
             "tar -czf /sdcard/Download/njm_2_ekosustav_01_clean_%s.tar.gz -C \"%s\" njm_2_ekosustav_01",
             timestamp, home);

    if (exec_step("Izrada sigurnosne kopije u /sdcard/Download", backup_cmd) != 0) {
        return EXIT_FAILURE;
    }

    /* 2. Brisanje starih backup referenci ako postoje iz prijašnjih pokušaja */
    system("rm -rf .git/refs/original/");

    /* 3. Nativno uklanjanje registar_akata.json iz cjelokupne Git povijesti */
    const char *filter_cmd =
        "FILTER_BRANCH_SQUELCH_WARNING=1 git filter-branch --force --index-filter "
        "\"git rm --cached --ignore-unmatch registar_akata.json\" "
        "--prune-empty --tag-name-filter cat -- --all";

    if (exec_step("Prepisivanje Git povijesti bez registar_akata.json", filter_cmd) != 0) {
        return EXIT_FAILURE;
    }

    /* 4. Čišćenje lokalnih Git reflogova i pakiranje objekata */
    if (exec_step("Čišćenje reflog zapisa i sažimanje Git baze",
                  "rm -rf .git/refs/original/ && git reflog expire --expire=now --all && git gc --prune=now --aggressive") != 0) {
        return EXIT_FAILURE;
    }

    /* 5. Provjera postoji li još ikakav trag datoteke u povijesti */
    printf("\n[C23 PROVJERA] Pretraživanje povijesti za 'registar_akata.json'...\n");
    int check_rc = system("test -z \"$(git log --all --oneline -- registar_akata.json)\"");
    if (check_rc != 0) {
        fprintf(stderr, "[C23 BLOKADA] Trag datoteke još uvijek postoji u povijesti! Prekidam push.\n");
        return EXIT_FAILURE;
    }
    printf("[C23 POTVRDA] Povijest je 100%% čista.\n");

    /* 6. Slanje čiste povijesti na službeni GitHub repozitorij */
    if (exec_step("Force push čiste povijesti na origin/main", "git push origin main --force") != 0) {
        return EXIT_FAILURE;
    }

    printf("\n[C23 TRIJUMF] Sigurnosna arhiva spremljena, Git povijest očišćena i objavljena!\n");
    return EXIT_SUCCESS;
}
