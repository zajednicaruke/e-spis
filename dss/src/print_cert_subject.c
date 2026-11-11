#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "qes.h"

int main(void) {
    const char *pdf_path = "/sdcard/01 HEP/Prigovor Pl-249-2026-3 HEP.pdf";
    
    QES_Info qes_info;
    memset(&qes_info, 0, sizeof(qes_info));

    printf("[OK] Pokrećem analizu QES potpisa za: %s\n", pdf_path);

    int qes_res = qes_analiziraj(pdf_path, &qes_info);

    printf("\n================ KRIPTOGRAFSKI REZULTAT U C23 ================\n");
    if (qes_res == 1) {
        printf("QES Status: VALIDAN POTPIS\n");
        // Ispisujemo polja prema standardnoj QES_Info strukturi
        printf("Potpisnik: %s\n", qes_info.potpisnik[0] ? qes_info.potpisnik : "N/A");
    } else {
        printf("QES Status: Nije pronađen prepoznatljiv QES profil ili nije potpisan.\n");
    }
    printf("============================================================\n");

    return 0;
}
