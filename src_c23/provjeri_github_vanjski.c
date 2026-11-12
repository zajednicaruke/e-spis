#include <stdio.h>
#include <stdlib.h>

int main(void) {
    printf("=== C23 VANJSKA PROVJERA GITHUBA I PAGESA ===\n");
    
    printf("[1] GitHub Pages - registar_akata.json (mora biti 404): ");
    fflush(stdout);
    system("curl -s -o /dev/null -w '%{http_code}\\n' https://zajednicaruke.github.io/e-spis/registar_akata.json");

    printf("[2] Broj forkova repozitorija (mora biti 0): ");
    fflush(stdout);
    system("curl -s https://api.github.com/repos/zajednicaruke/e-spis/forks | grep -c '\"id\"' || true");

    printf("[3] Broj Pull Requestova (mora biti 0): ");
    fflush(stdout);
    system("curl -s 'https://api.github.com/repos/zajednicaruke/e-spis/pulls?state=all' | grep -c '\"id\"' || true");

    printf("[4] Broj Issue prijava (mora biti 0): ");
    fflush(stdout);
    system("curl -s 'https://api.github.com/repos/zajednicaruke/e-spis/issues?state=all' | grep -c '\"id\"' || true");

    return EXIT_SUCCESS;
}
