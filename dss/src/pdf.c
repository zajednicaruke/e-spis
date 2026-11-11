#include "pdf.h"
#include "util.h"
#include "font_mod.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

static size_t hex_to_bin_pdf(const uint8_t *hex_start, const uint8_t *hex_end, uint8_t *out_buf, size_t out_max) {
    size_t out_idx = 0;
    const uint8_t *q = hex_start;
    int high_nibble = -1;

    while (q < hex_end && *q != '>') {
        uint8_t c = *q;
        if (c == ' ' || c == '\r' || c == '\n' || c == '\t' || c == '\0') {
            q++;
            continue;
        }

        int val = -1;
        if (c >= '0' && c <= '9') val = c - '0';
        else if (c >= 'a' && c <= 'f') val = c - 'a' + 10;
        else if (c >= 'A' && c <= 'F') val = c - 'A' + 10;

        if (val >= 0) {
            if (high_nibble == -1) {
                high_nibble = val;
            } else {
                if (out_idx < out_max) {
                    out_buf[out_idx++] = (uint8_t)((high_nibble << 4) | val);
                }
                high_nibble = -1;
            }
        }
        q++;
    }
    return out_idx;
}

int pdf_extract_signature_data(const uint8_t *pdf_buf, size_t pdf_len, PdfSignatureData *out_data) {
    if (!pdf_buf || pdf_len == 0 || !out_data) return 0;
    memset(out_data, 0, sizeof(PdfSignatureData));

    const uint8_t *search_ptr = pdf_buf;
    size_t remaining_len = pdf_len;

    while (remaining_len > 9) {
        const void *contents_ptr = memmem_case(search_ptr, remaining_len, "/Contents", 9);
        if (!contents_ptr) break;

        const uint8_t *p = (const uint8_t*)contents_ptr + 9;
        const uint8_t *end = pdf_buf + pdf_len;

        while (p < end && *p != '<' && *p != '(') p++;
        if (p < end && *p == '<') {
            p++;
            static uint8_t sig_storage[32768];
            size_t extracted_sig_len = hex_to_bin_pdf(p, end, sig_storage, sizeof(sig_storage));

            if (extracted_sig_len > 30) {
                out_data->sig_bytes = sig_storage;
                out_data->sig_len = extracted_sig_len;
                out_data->cert_der = sig_storage;
                out_data->cert_len = extracted_sig_len;
                return 1;
            }
        }

        size_t offset = ((const uint8_t*)contents_ptr - search_ptr) + 9;
        search_ptr = (const uint8_t*)contents_ptr + 9;
        if (offset >= remaining_len) break;
        remaining_len -= offset;
    }

    return 0;
}

void pdf_generiraj(const char *putanja, const char *tekst_izvjestaja, size_t tekst_len) {
    (void)tekst_len;
    FILE *f = fopen(putanja, "wb");
    if (!f) return;

    // Učitaj Roboto font iz sustava
    unsigned char ttf_buf[4194304];
    size_t ttf_len = 0;
    FILE *fttf = fopen("/system/fonts/Roboto-Regular.ttf", "rb");
    if (!fttf) fttf = fopen("/system/fonts/DroidSans.ttf", "rb");
    if (fttf) {
        fseek(fttf, 0, SEEK_END);
        long sz = ftell(fttf);
        fseek(fttf, 0, SEEK_SET);
        if (sz > 0 && (size_t)sz < sizeof(ttf_buf)) {
            ttf_len = fread(ttf_buf, 1, sz, fttf);
        }
        fclose(fttf);
    }

    static char stream[131072];
    int offset = 0;

    offset += sprintf(stream + offset, "BT\n/F1 9 Tf\n12 TL\n36 780 Td\n");

    const char *line = tekst_izvjestaja;
    while (*line) {
        const char *next = strchr(line, '\n');
        size_t len = next ? (size_t)(next - line) : strlen(line);
        char clean_line[512];
        if (len >= sizeof(clean_line)) len = sizeof(clean_line) - 1;
        strncpy(clean_line, line, len);
        clean_line[len] = '\0';

        if (ttf_len > 0) {
            char hex_line[2048];
            int hex_len = font_mod_utf8_to_utf16be_hex(clean_line, hex_line, sizeof(hex_line));
            if (hex_len > 0) {
                offset += sprintf(stream + offset, "<%s> Tj T*\n", hex_line);
            } else {
                offset += sprintf(stream + offset, "T*\n");
            }
        } else {
            char translit_line[512];
            font_mod_transliterate(clean_line, translit_line, sizeof(translit_line));
            if (translit_line[0] == '\0') {
                offset += sprintf(stream + offset, "T*\n");
            } else {
                offset += sprintf(stream + offset, "(%s) Tj T*\n", translit_line);
            }
        }

        if (!next) break;
        line = next + 1;
    }

    offset += sprintf(stream + offset, "ET\n");

    long xref_offsets[10];
    int obj_idx = 1;

    fprintf(f, "%%PDF-1.4\n");

    // 1 0 obj - Catalog
    xref_offsets[obj_idx++] = ftell(f);
    fprintf(f, "1 0 obj\n<</Type/Catalog/Pages 2 0 R>>\nendobj\n");

    // 2 0 obj - Pages
    xref_offsets[obj_idx++] = ftell(f);
    fprintf(f, "2 0 obj\n<</Type/Pages/Count 1/Kids[3 0 R]>>\nendobj\n");

    // 3 0 obj - Page
    xref_offsets[obj_idx++] = ftell(f);
    fprintf(f, "3 0 obj\n<</Type/Page/Parent 2 0 R/Resources<</Font<</F1 4 0 R>>>>/MediaBox[0 0 595 3000]/Contents 5 0 R>>\nendobj\n");

    if (ttf_len > 0) {
        // 4 0 obj - Type0 Font
        xref_offsets[obj_idx++] = ftell(f);
        fprintf(f, "4 0 obj\n<</Type/Font/Subtype/Type0/BaseFont/Roboto-Regular/Encoding/Identity-H/DescendantFonts[6 0 R]/ToUnicode 9 0 R>>\nendobj\n");

        // 5 0 obj - Contents Stream
        xref_offsets[obj_idx++] = ftell(f);
        fprintf(f, "5 0 obj\n<</Length %d>>\nstream\n%s\nendstream\nendobj\n", offset, stream);

        // 6 0 obj - CIDFont
        xref_offsets[obj_idx++] = ftell(f);
        fprintf(f, "6 0 obj\n<</Type/Font/Subtype/CIDFontType2/BaseFont/Roboto-Regular/CIDSystemInfo<</Registry(Adobe)/Ordering(Identity)/Supplement 0>>/FontDescriptor 7 0 R/DW 1000/CIDToGIDMap/Identity>>\nendobj\n");

        // 7 0 obj - FontDescriptor
        xref_offsets[obj_idx++] = ftell(f);
        fprintf(f, "7 0 obj\n<</Type/FontDescriptor/FontName/Roboto-Regular/Flags 32/ItalicAngle 0/Ascent 928/Descent -236/CapHeight 729/BBox[-500 -300 1200 1000]/FontFile2 8 0 R>>\nendobj\n");

        // 8 0 obj - FontFile2 (TTF binary)
        xref_offsets[obj_idx++] = ftell(f);
        fprintf(f, "8 0 obj\n<</Length %zu>>\nstream\n", ttf_len);
        fwrite(ttf_buf, 1, ttf_len, f);
        fprintf(f, "endstream\nendobj\n");

        // 9 0 obj - ToUnicode CMap (dinamička duljina)
        xref_offsets[obj_idx++] = ftell(f);
        const char *cmap_content =
            "/CIDInit /ProcSet findresource begin\n"
            "12 dict begin\n"
            "begincmap\n"
            "/CIDSystemInfo <</Registry (Adobe) /Ordering (Identity) /Supplement 0>> def\n"
            "/CMapName /Custom-CMap def\n"
            "/CMapType 2 def\n"
            "1 begincodespacerange\n"
            "<0000> <FFFF>\n"
            "endcodespacerange\n"
            "1 beginbfrange\n"
            "<0000> <FFFF> <0000>\n"
            "endbfrange\n"
            "endcmap\n"
            "CMapName currentdict /CMap defineresource pop\n"
            "end\n"
            "end\n";
        int cmap_len = strlen(cmap_content);
        fprintf(f, "9 0 obj\n<</Length %d>>\nstream\n%s\nendstream\nendobj\n", cmap_len, cmap_content);
    } else {
        // Fallback Type1 Helvetica
        xref_offsets[obj_idx++] = ftell(f);
        fprintf(f, "4 0 obj\n<</Type/Font/Subtype/Type1/BaseFont/Helvetica/Encoding/WinAnsiEncoding>>\nendobj\n");

        xref_offsets[obj_idx++] = ftell(f);
        fprintf(f, "5 0 obj\n<</Length %d>>\nstream\n%s\nendstream\nendobj\n", offset, stream);
    }

    int total_objs = ttf_len > 0 ? 9 : 5;
    long startxref = ftell(f);

    fprintf(f, "xref\n0 %d\n", total_objs + 1);
    fprintf(f, "0000000000 65535 f \n");
    for (int i = 1; i <= total_objs; i++) {
        fprintf(f, "%010ld 00000 n \n", xref_offsets[i]);
    }

    fprintf(f, "trailer\n<</Size %d/Root 1 0 R>>\nstartxref\n%ld\n%%%%EOF\n", total_objs + 1, startxref);
    fclose(f);
}
