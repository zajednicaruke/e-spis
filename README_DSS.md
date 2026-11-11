# DSSvalidator — C23 QES PDF Validator (v5.0)

**Projekt:** Nulto-ovisni C23 QES PDF validator za Institut Zajednice RUKE (OIB: 64517735227)
**Okruženje:** Termux / Android AArch64 (Samsung Galaxy A53, Exynos 1280)
**Jezik:** C23 (Clang, bez vanjskih ovisnosti, samo `libc`)
**Verzija:** v5.0

---

## ✅ Postignute tehničke značajke

- **C23 nativni build** — `-std=c2x -O3`, 0 warninga, 40 KB binarna datoteka
- **Pravi hrvatski dijakritici** — Ugrađen `Roboto-Regular.ttf` font s `Identity-H` i UTF-16BE kodiranjem (`č`, `ć`, `đ`, `ž`, `š` u PDF nalazu)
- **0 Syntax Error** — PDF je usklađen s PDF 1.4 specifikacijom
- **QES Extractor** — Automatska detekcija i ekstrakcija metapodataka potpisnika (AKD, Fina) iz X.509 DER certifikata
- **Zbirni integritet** — SHA-256 izračuni za sve akte u mapi

## ⚠️ Djelomično implementirano

- **QES razina 3 (kripto verify)** — `rsa.c` i `ecdsa_p256.c` postoje, ali `DSSvalidator` ih ne poziva
- **CA lanac** — nije implementirano
- **OCSP/CRL** — nije implementirano
- **TSA (vremenski žig)** — nije implementirano

## ❌ Što NE radi

- Ne verificira kriptografsku valjanost potpisa
- Ne provjerava CA lanac
- Ne provjerava OCSP/CRL
- Ne verificira kvalificirani status QES-a

## 🚀 Korištenje

DSSvalidator <<< "01 HEP"

Generira: /sdcard/01 HEP/Nalaz_Validacije_Zajednica_RUKE.pdf

## 📅 Verzija

- **v5.0** — 11.11.2026.
