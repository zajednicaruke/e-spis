#ifndef DER_H
#define DER_H
#include <stddef.h>
#include <stdint.h>

typedef struct {
    int   uspjesno;
    char  cn[128];
    char  o[128];
    char  ou[128];
    char  c[8];
    char  oib[32];
    char  serial[64];
    char  izdavatelj_cn[128];
    char  izdavatelj_o[128];
    char  not_before[32];
    char  not_after[32];
    char  algoritam[64];
    size_t der_velicina;
} DER_Cert;

int  der_izvuci_certifikat(const unsigned char *pdf_buf, size_t pdf_len,
                           DER_Cert *out);
void der_formatiraj(const DER_Cert *cert, char *out, size_t outlen);

#endif

/* Parsiraj raw DER certifikat (bez PDF omotaca).
   Vraca 1 ako je uspjesno, 0 ako nije. */
int der_parsiraj_der_cert(const unsigned char *cert, size_t cert_len,
                           DER_Cert *out);
