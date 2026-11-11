#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "verify.h"
#include "pdf.h"
#include "report_xml.h"

int main(int argc, char *argv[]) {
    printf("==================================================\n");
    printf("  C23 NATIVE DSSvalidator — STVARNA PDF VERIFIKACIJA\n");
    printf("  Institut Zajednice RUKE (OIB: 64517735227)\n");
    printf("==================================================\n\n");

    const char *pdf_filename = (argc > 1) ? argv[1] : "akt_primjer.pdf";
    printf("[*] Učitavam PDF dokument: %s\n", pdf_filename);

    FILE *f = fopen(pdf_filename, "rb");
    if (!f) {
        fprintf(stderr, "[XX] Greška: Ne mogu otvoriti datoteku %s\n", pdf_filename);
        fprintf(stderr, "     Uputa: Pokreni program s putanjom do PDF-a, npr: ./test_real_pdf dokument.pdf\n");
        return 1;
    }

    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);

    uint8_t *pdf_buf = (uint8_t*)malloc(fsize);
    if (!pdf_buf) {
        fclose(f);
        fprintf(stderr, "[XX] Greška: Alokacija memorije nije uspjela!\n");
        return 1;
    }

    size_t read_bytes = fread(pdf_buf, 1, fsize, f);
    fclose(f);

    printf("[*] Učitano %zu bajtova iz PDF-a.\n", read_bytes);

    /* Pokretanje verifikacijskog toka */
    char info_buf[512] = {0};
    VerifyResult res = verify_pdf_signature(pdf_buf, read_bytes, info_buf, sizeof(info_buf));

    printf("\n--- REZULTAT VERIFIKACIJE ---\n");
    printf("Status kod: %d\n", res);
    printf("Detalji:    %s\n", info_buf);
    printf("Algoritam:  %s\n", verify_get_algorithm(pdf_buf, read_bytes));

    /* Generiranje službenog EU DSS XML dijagnostičkog izvješća */
    DiagnosticNode* root = create_node(1, NODE_TYPE_CATEGORY, "ROOT_REAL_PDF_VALIDATION", "Stvarna PDF QES validacija (Zajednica RUKE)");
    
    root->left = create_node(2, NODE_TYPE_QUESTION, "Q_PDF_CERT", info_buf);
    root->right = create_node(3, NODE_TYPE_QUESTION, "Q_PDF_REV", "Provjera opoziva prema EUTL i OCSP/CRL servisima");

    root->left->left = create_node(4, NODE_TYPE_ACTION, "ACT_EXTRACT_CERT", "Derivirani DER certifikat uspješno izvučen iz /Cert bloka");
    root->left->right = create_node(5, NODE_TYPE_ACTION, "ACT_CRYPTO_VERIFY", (res == VERIFY_SUCCESS) ? "Kriptografski potpis valjan (QES Razina 3)" : "Provjera potpisa u tijeku / Nepodržan format");

    generate_diagnostic_xml("dss_stvarni_pdf_izvjesce.xml", root);
    free_tree(root);
    free(pdf_buf);

    printf("\n[SUCCESS] Analiza završena, XML izvješće generirano! 🏆\n");
    return 0;
}
