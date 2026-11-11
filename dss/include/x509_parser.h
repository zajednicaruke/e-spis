#ifndef X509_PARSER_H
#define X509_PARSER_H

#include <stdint.h>
#include <stddef.h>

typedef struct {
    char subject_name[128];
    char issuer_name[128];
    char oib[32];
    char serial_number[64];
    int is_qualified; /* QES oznaka */
} X509CertInfo;

/* Funkcija za parsiranje X.509 certifikata iz DER bajtova */
int x509_parse_cert(const uint8_t *der_buf, size_t der_len, X509CertInfo *info);

#endif // X509_PARSER_H
