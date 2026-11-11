#ifndef PDF_H
#define PDF_H

#include <stdint.h>
#include <stddef.h>

/* Struktura za spremanje pronađenih binarnih dijelova potpisa iz PDF-a */
typedef struct {
    const uint8_t *cert_der;
    size_t cert_len;
    const uint8_t *sig_bytes;
    size_t sig_len;
} PdfSignatureData;

/* Pronalazi i izdvaja certifikat i potpis iz PDF bajtova */
int pdf_extract_signature_data(const uint8_t *pdf_buf, size_t pdf_len, PdfSignatureData *out_data);

#endif // PDF_H
