#ifndef VERIFY_H
#define VERIFY_H

#include <stdint.h>
#include <stddef.h>

typedef enum {
    VERIFY_SUCCESS = 0,
    VERIFY_NO_SIG,
    VERIFY_NO_PUBKEY,
    VERIFY_INVALID_SIG,
    VERIFY_UNSUPPORTED
} VerifyResult;

VerifyResult verify_pdf_signature(const uint8_t *pdf_buf, size_t pdf_len,
                                   char *info_out, size_t info_len);

const char *verify_get_algorithm(const uint8_t *pdf_buf, size_t pdf_len);

#endif // VERIFY_H
