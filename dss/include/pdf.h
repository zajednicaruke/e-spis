#ifndef PDF_H
#define PDF_H

#include <stddef.h>
#include <stdint.h>

typedef struct {
    const uint8_t *sig_bytes;
    size_t sig_len;
    const uint8_t *cert_der;
    size_t cert_len;
} PdfSignatureData;

int pdf_extract_signature_data(const uint8_t *pdf_buf, size_t pdf_len, PdfSignatureData *out_data);
void pdf_generiraj(const char *putanja, const char *tekst_izvjestaja, size_t tekst_len);

#endif

/* ============================================================
 * P1.1 — /ByteRange parser i rekonstrukcija potpisanog sadržaja
 * ============================================================ */

typedef struct {
    size_t range1_start;   
    size_t range1_len;     
    size_t range2_start;   
    size_t range2_len;     
    int    valjan;         
} PdfByteRange;

int pdf_extract_byterange(const uint8_t *pdf_buf, size_t pdf_len,
                           PdfByteRange *out);

int pdf_reconstruct_signed_content(const uint8_t *pdf_buf, size_t pdf_len,
                                    const PdfByteRange *br,
                                    uint8_t **out, size_t *out_len);
