#ifndef X509_PARSER_H
#define X509_PARSER_H

#include <stdint.h>
#include <stddef.h>

typedef struct {
    char subject_name[128];
    char subject_o[128];
    char oib[32];
    char issuer_name[128];
    char serial_number[64];
    char not_before[32];
    char not_after[32];
    char algorithm[64];
    int is_qualified;
} X509CertInfo;

int x509_parse_cert(const uint8_t *der_buf, size_t der_len, X509CertInfo *info);

#endif // X509_PARSER_H
