
# README.md — Ekosustav Digitalnog Pečata Zajednice RUKE

**Projekt:** Decentralizirani verifikacijski sustav digitalnog pečata i registra akata
**Pravni subjekt:** Udruga ZAJEDNICA RUKE (OIB: 64517735227, Delenšice 11, Zagreb)
**Okruženje:** Termux / Android AArch64 (`~/njm_2_ekosustav_01`)
**Jezik i standard:** C23 (Clang, OpenSSL EVP API, libqrencode)
**Trošak implementacije:** 0,00 EUR

## 📁 Registar i struktura direktorija (`~/njm_2_ekosustav_01`)

```text
~/njm_2_ekosustav_01/
├── pečat_zajednica_RUKE.c         # C23 izvorni kôd sustava (OpenSSL EVP + libqrencode + JSON engine)
├── pečat_zajednica_RUKE           # Kompajlirani AArch64 izvršni binarni modul
├── registar_akata.json            # Unificirana JSON baza/registar svih verificiranih akata
├── index.html                     # Dinamički generirani statički verifikacijski web portal
└── QR_*.pbm                       # Generirane PBM matrične slike QR kodova
```

### Detaljni specifikator datoteka:
| Datoteka | Vrsta / Format | Uloga u ekosustavu |
|---|---|---|
| `pečat_zajednica_RUKE.c` | C23 Source | Računa SHA-256 preko OpenSSL EVP API-ja, generira XXXX-XXXX kod, PBM QR sliku i regenerira index.html. |
| `pečat_zajednica_RUKE` | ELF 64-bit | Kompajlirana izvršna datoteka (`Clang -std=c23 -lcrypto -lqrencode`). |
| `registar_akata.json` | JSON Database | Struktura baze s fiksiranim zaglavljem udruge i zapisima akata. |
| `index.html` | HTML5 / JS | Klijentski verifikacijski portal s podlogom za Zero-Touch URL parametre. |
| `QR_*.pbm` | PBM Raster | 1-bitni raster QR koda spreman za ugradnju na vizualni sloj dokumenta. |

## 🎯 Definirani ciljevi projekta
1. **Digitalna suverenost i nulti trošak (0 €):** Eliminacija pretplata i komercijalnih posrednika.
2. **Supstitucija klasičnog pečata "Digitalnim sidrom":** Spajanje osobnog Certilia QES potpisa s SHA-256 otiskom akta i registrom udruge.
3. **Autonomni e-Spis model verifikacije:** Dvorazinska provjera izvornosti putem Broja akta i Kontrolnog koda (`XXXX-XXXX`).
4. **Zero-Touch automatizacija verifikacije:** Izravno skenersko povezivanje QR koda s verifikacijskim portalom.

## 🚀 Postignuta arhitektura i verifikacijski lanac
* **Identitet:** Certilia QES certifikat jamči pravni status ovlaštene osobe.
* **Kontinuitet:** Poziv na registar i unificirani broj akta dokazuje kontinuitet poslovanja Zajednice RUKE.
* **Integritet:** OpenSSL EVP SHA-256 algoritam fiksira stanje dokumenta bit-po-bit.

### Trenutačni status indeksiranih akata u registru:
* `MG/HEP-05-11-2026` ➔ Kontrolni broj: **E9A2-ECC3**
* `MG/KANGOO-ZAHTJEV-2026` ➔ Kontrolni broj: **B9E8-51F5**
* `MG/KANGOO-PUNOMOC-2026` ➔ Kontrolni broj: **589F-1771**

---

## ⚖️ 1:1 Usporedba s pravosudnim sustavom (e-Spis / e-Komunikacija)

| Pravosudni e-Spis | C23 sustav (`pečat_zajednica_RUKE`) | Uloga u arhitekturi |
|---|---|---|
| **Broj zapisa** (`9-3087a-c516f`) | **broj_akta** (`MG/KANGOO-PUNOMOC-2026`) | Primarni indeksni ključ za dohvat u bazi |
| **Kontrolni broj** (`01069-ca838-209dc`) | **kontrolni_broj** (`589F-1771`) | Formatirani heksadekadski blok SHA-256 sažetka |
| **Pravna valjanost** | **Certilia QES potpis** | eIDAS pravno izjednačenje s vlastoručnim potpisom |
| **Javna provjera** | `e-komunikacija.pravosudje.hr` | Statički verifikacijski portal `index.html` |
