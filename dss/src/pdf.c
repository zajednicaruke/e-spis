#include "pdf.h"
#include "util.h"
#include <string.h>
#include <stdio.h>

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

            if (extracted_sig_len > 100) {
                out_data->sig_bytes = sig_storage;
                out_data->sig_len = extracted_sig_len;
                out_data->cert_der = sig_storage; // Koristimo izvučeni PKCS#7 kontejner koji sadrži certifikat
                out_data->cert_len = extracted_sig_len;
                return 1;
            }
        }

        size_t offset = ((const uint8_t*)contents_ptr - search_ptr) + 9;
        search_ptr = (const uint8_t*)contents_ptr + 9;
        remaining_len -= offset;
    }

    return 0;
}
