#ifndef QES_H
#define QES_H
#include <stddef.h>
#include "der.h"
#include "akd.h"

typedef struct {
    int   potpisan;              /* 1 = QES potpis prisutan */
    char  subfilter[64];         /* ETSI.CAdES.detached, adbe.pkcs7.detached... */
    char  filter[64];            /* Adobe.PPKLite... */
    char  potpisnik[128];        /* iz /Name ili DER CN */
    char  vrijeme[64];           /* /M(D:...) */
    char  byte_range[128];       /* [0 182116 234118 25561] */
    char  razina[32];            /* "QES (CAdES)", "PKCS7" itd. */

    /* NOVO: DER X.509 certifikat */
    DER_Cert cert;

    /* NOVO: AKD Certilia widget metapodaci */
    AKD_Meta akd;
} QES_Info;

int  qes_analiziraj(const char *filepath, QES_Info *info);
void qes_formatiraj(const QES_Info *info, char *out, size_t outlen);

#endif
