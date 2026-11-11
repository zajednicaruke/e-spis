#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "pdf.h"

int generiraj_pdf_report(const char *putanja, const char *folder, int ukupno, long bajtova, const char *zbirni_hash) {
    FILE *f = fopen(putanja, "w");
    if (!f) return 0;

    char stream[8192];
    int len = 0;

    len += sprintf(stream + len, "BT\n/F1 10 Tf\n14 TL\n36 780 Td\n");
    len += sprintf(stream + len, "(SLUZBENI NALAZ VALIDACIJE E-SPISA) Tj T*\n");
    len += sprintf(stream + len, "(Institut Zajednice RUKE - OIB: 64517735227) Tj T*\n");
    len += sprintf(stream + len, "(Ciljani folder: %s) Tj T*\n", folder);
    len += sprintf(stream + len, "(Ukupno dokumenata: %d | Velicina: %ld B) Tj T*\n", ukupno, bajtova);
    len += sprintf(stream + len, "(Zbirni SHA-256: %s) Tj T*\n", zbirni_hash);
    len += sprintf(stream + len, "(-----------------------------------------------------------------) Tj T*\n");
    len += sprintf(stream + len, "ET\n");

    fprintf(f, "%%PDF-1.4\n");
    fprintf(f, "1 0 obj\n<</Type/Catalog/Pages 2 0 R>>\nendobj\n");
    fprintf(f, "2 0 obj\n<</Type/Pages/Count 1/Kids[3 0 R]>>\nendobj\n");
    fprintf(f, "3 0 obj\n<</Type/Page/Parent 2 0 R/Resources<</Font<</F1 4 0 R>>>>/MediaBox[0 0 595 842]/Contents 5 0 R>>\nendobj\n");
    fprintf(f, "4 0 obj\n<</Type/Font/Subtype/Type1/BaseFont/Helvetica/Encoding/WinAnsiEncoding>>\nendobj\n");
    fprintf(f, "5 0 obj\n<</Length %d>>\nstream\n%s\nendstream\nendobj\n", len, stream);

    long o1 = 9, o2 = 58, o3 = 115, o4 = 243, o5 = 342;
    long startxref = o5 + 40 + len;

    fprintf(f, "xref\n0 6\n0000000000 65535 f \n");
    fprintf(f, "%010ld 00000 n \n%010ld 00000 n \n%010ld 00000 n \n%010ld 00000 n \n%010ld 00000 n \n", o1, o2, o3, o4, o5);
    fprintf(f, "trailer\n<</Size 6/Root 1 0 R>>\nstartxref\n%ld\n%%%%EOF\n", startxref);

    fclose(f);
    return 1;
}
