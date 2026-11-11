#ifndef AKD_H
#define AKD_H
#include <stddef.h>

/* ============================================================
 * AKD Certilia "Signature Widget" parser
 *
 * AKD ePotpis u PDF-u ostavlja čitljive metapodatke u formatu:
 *   /akpid-XXX#23A010Zaglavlje(...)
 *   /akpid-XXX#23A030VrstaPotpisa(SIGN)
 *   /akpid-XXX#23A040DatumVrijemePotpisa(19.10.2026T11:38:09 ...)
 *   /akpid-XXX#23A060Potpisnik(ime)
 *   /akpid-XXX#23A070PotpisnikPIN(HR-05315840697)
 *   /akpid-XXX#23A110Provjera(https://esign.certilia.com/...)
 * ============================================================ */

typedef struct {
    int  pronaden;              /* 1 = AKD widget prisutan */
    char zaglavlje[256];        /* "AKD ePotpis PAdES2 2.0.9779..." */
    char broj_zapisa[64];       /* UUID */
    char vrsta_potpisa[32];     /* "SIGN" */
    char datum_potpisa[64];     /* "19.10.2026T11:38:09 (UTC:...)" */
    char potpisnik[128];        /* ime i prezime */
    char potpisnik_pin[32];     /* "HR-05315840697" */
    char provjera_url[256];     /* URL provjere */
    char izdavatelj[128];       /* izdavatelj certifikata */
    char podnozje[128];         /* "AKD ePotpis" */
} AKD_Meta;

/* Izvuci AKD Certilia metapodatke iz PDF-a.
   Vraća 1 ako je widget pronađen, 0 ako nije. */
int akd_izvuci_meta(const unsigned char *pdf_buf, size_t pdf_len,
                    AKD_Meta *out);

/* Formatiraj AKD_Meta u čitljiv string */
void akd_formatiraj(const AKD_Meta *meta, char *out, size_t outlen);

#endif

