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
