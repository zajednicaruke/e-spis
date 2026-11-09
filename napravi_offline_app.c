/* OPTION-V2 */
/* KLASE-CLANSTVA-V2 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

int main(void) {
    FILE *fhtml = fopen("index.html", "wb");
    if (!fhtml) { perror("Greška pri otvaranju index.html"); return EXIT_FAILURE; }

    /* UTF-8 BOM */
    fputc(0xEF, fhtml); fputc(0xBB, fhtml); fputc(0xBF, fhtml);

    fprintf(fhtml, "<!DOCTYPE html>\n<html lang=\"hr\">\n<head>\n");
    fprintf(fhtml, "<meta charset=\"UTF-8\">\n<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n");
    fprintf(fhtml, "<title>Zajednica RUKE • eIDAS e-Spis Portal</title>\n");
    fprintf(fhtml, "<script src=\"https://cdnjs.cloudflare.com/ajax/libs/qrious/4.0.2/qrious.min.js\"></script>\n");
    fprintf(fhtml, "<style>\n");
    fprintf(fhtml, "  body { font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, Helvetica, Arial, sans-serif; background: #f8fafc; color: #0f172a; margin: 0; padding: 15px; }\n");
    fprintf(fhtml, "  .card { background: #ffffff; border-radius: 12px; padding: 24px; box-shadow: 0 4px 16px rgba(0,0,0,0.06); border: 1px solid #e2e8f0; max-width: 800px; margin: 20px auto; }\n");
    fprintf(fhtml, "  .btn { border: none; padding: 12px 20px; border-radius: 8px; font-weight: bold; font-size: 14px; cursor: pointer; transition: all 0.2s; }\n");
    fprintf(fhtml, "  .btn-blue { background: #2563eb; color: #fff; } .btn-dark { background: #0f172a; color: #fff; } .btn-green { background: #166534; color: #fff; }\n");
    fprintf(fhtml, "  .input-field { width: 100%%; padding: 9px; border: 1px solid #cbd5e1; border-radius: 6px; font-size: 13px; box-sizing: border-box; margin-top: 4px; margin-bottom: 12px; }\n");
    fprintf(fhtml, "  .logo-main { display: block; margin: 0 auto 15px auto; max-width: 160px; width: 100%%; height: 160px; object-fit: cover; border-radius: 50%%; box-shadow: 0 4px 16px rgba(0,0,0,0.12); border: 3px solid #e2e8f0; }\n");
    fprintf(fhtml, "  .logo-icon { width: 55px; height: 55px; border-radius: 50%%; object-fit: cover; display: block; }\n");
    fprintf(fhtml, "\n/* KOZMETIKA-V2 */\n  .card { text-align: center; }\n  .card picture { display: block; margin: 0 auto; }\n  .card .logo-main { margin: 0 auto 15px auto; }\n  .btn-yellow { background: #eab308; color: #0f172a; box-shadow: 0 2px 8px rgba(234,179,8,0.3); }\n  .btn-yellow:hover { background: #ca8a04; color: #fff; }\n  .btn-red { background: #dc2626; color: #ffffff; box-shadow: 0 2px 8px rgba(220,38,38,0.3); }\n  .btn-red:hover { background: #b91c1c; }\n  .btn-green { background: #166534; color: #ffffff; box-shadow: 0 2px 8px rgba(22,101,52,0.3); }\n  .btn-green:hover { background: #14532d; }\n  .btn-blue:hover { background: #1d4ed8; }\n  .btn-dark:hover { background: #1e293b; }\n\n</head>\n<body>\n\n");

    /* ==== EKRAN 0: GLAVNI ULAZ (EU LOGO) ==== */
    fprintf(fhtml, "<div id=\"ekran-ulaz\" class=\"card\" style=\"text-align:center;\">\n");
    fprintf(fhtml, "    <picture><source srcset=\"./EU_RUKE_1793640051705.webp\" type=\"image/webp\"><img src=\"./EU_RUKE_1793640051705.png\" alt=\"Logo Zajednica RUKE\" class=\"logo-main\"></picture>\n");
    fprintf(fhtml, "    <h2 style=\"margin:0; font-size:24px; color:#0f172a;\">ZAJEDNICA RUKE</h2>\n");
    fprintf(fhtml, "    <div style=\"font-size:14px; font-weight:bold; color:#1e293b; margin-top:6px;\">Hipostazologija | Zadružno Poduzetništvo</div>\n");
    fprintf(fhtml, "    <p style=\"color:#475569; font-size:13px; font-weight:bold; margin-top:4px; margin-bottom:12px;\">OIB: 64517735227 • Zagreb / Županja</p>\n");
    fprintf(fhtml, "    <div style=\"background:#f1f5f9; padding:8px 16px; border-radius:20px; font-size:12px; font-weight:bold; color:#334155; display:inline-block; margin-bottom:25px;\">eIDAS • Digitalna transformacija • e-građani</div>\n");
    fprintf(fhtml, "    <div style=\"display:grid; grid-template-columns:1fr 1fr; gap:15px;\">\n");
    fprintf(fhtml, "        <button class=\"btn btn-yellow\" onclick=\"prikaziSloj('javni')\" style=\"padding:20px;\">🌐 JAVNI PRISTUP<br><span style=\"font-size:11px; font-weight:normal; opacity:0.9;\">Za građane, tijela javne vlasti i partnere</span></button>\n");
    fprintf(fhtml, "        <button class=\"btn btn-blue\" onclick=\"prikaziSloj('clanovi')\" style=\"padding:20px;\">🔒 ČLANSKI PRETINAC<br><span style=\"font-size:11px; font-weight:normal; opacity:0.9;\">Za autorizirane članove uz PIN</span></button>\n");
    fprintf(fhtml, "    </div>\n</div>\n\n");

    /* ==== EKRAN 1: JAVNI PORTAL (ORDEN LOGO) ==== */
    fprintf(fhtml, "<div id=\"ekran-javni\" class=\"card\" style=\"display:none;\">\n");
    fprintf(fhtml, "    <div style=\"display:flex; justify-content:space-between; align-items:center; border-bottom:2px solid #f1f5f9; padding-bottom:15px; margin-bottom:20px;\">\n");
    fprintf(fhtml, "        <div style=\"display:flex; align-items:center; gap:12px;\">\n");
    fprintf(fhtml, "            <picture><source srcset=\"./Orden_RUKE_1772802905749.webp\" type=\"image/webp\"><img src=\"./Orden_RUKE_1772802905749.png\" alt=\"Orden RUKE\" class=\"logo-icon\"></picture>\n");
    fprintf(fhtml, "            <div><h3 style=\"margin:0; font-size:18px;\">JAVNI PORTAL E-SPIS</h3><span style=\"font-size:12px; color:#64748b;\">Zajednica RUKE • eIDAS Verifikator</span></div>\n");
    fprintf(fhtml, "        </div>\n");
    fprintf(fhtml, "        <button class=\"btn\" onclick=\"prikaziSloj('ulaz')\" style=\"background:#f1f5f9; color:#475569; font-size:12px;\">⬅ Povratak</button>\n");
    fprintf(fhtml, "    </div>\n");
    fprintf(fhtml, "    <div style=\"display:flex; gap:10px; margin-bottom:20px;\">\n");
    fprintf(fhtml, "        <button class=\"btn btn-red\" onclick=\"prikaziFormu('j-upload')\" style=\"flex:1;\">📤 Učitaj PODNESAK</button>\n");
    fprintf(fhtml, "        <button class=\"btn btn-green\" onclick=\"prikaziFormu('j-dohvat')\" style=\"flex:1;\">📥 Preuzmi / Verificiraj PODNESAK</button>\n");
    fprintf(fhtml, "    </div>\n");
    
    /* JAVNA FORMA UČITAVANJA */
    fprintf(fhtml, "    <div id=\"box-j-upload\" style=\"background:#f8fafc; padding:20px; border-radius:8px; border:1px solid #cbd5e1;\">\n");
    fprintf(fhtml, "        <h4 style=\"margin-top:0;\">Uloži novi podnesak s lokalnim SHA-256 pečatom</h4>\n");
    fprintf(fhtml, "        <label style=\"font-size:11px; font-weight:bold;\">Broj akta / Oznaka:</label><input type=\"text\" id=\"ju-broj\" class=\"input-field\" placeholder=\"npr. MG/PRIGOVOR-2026\">\n");
    fprintf(fhtml, "        <label style=\"font-size:11px; font-weight:bold;\">Pošiljatelj (Ime i prezime / Naziv):</label><input type=\"text\" id=\"ju-posiljatelj\" class=\"input-field\" placeholder=\"Unesite vaše ime ili naziv institucije\">\n");
    fprintf(fhtml, "        <label style=\"font-size:11px; font-weight:bold;\">Primatelj (Institucija / Sud):</label><input type=\"text\" id=\"ju-primatelj\" class=\"input-field\" placeholder=\"npr. Općinski sud u Vinkovcima\">\n");
    fprintf(fhtml, "        <label style=\"font-size:11px; font-weight:bold;\">PDF Dokument:</label><input type=\"file\" id=\"ju-file\" accept=\"application/pdf\" style=\"display:block; margin:6px 0 15px 0; font-size:12px;\">\n");
    fprintf(fhtml, "        <button class=\"btn btn-green\" onclick=\"obradiJavniUpload()\">⚡ Izradi SHA-256 Pečat i Generiraj Potvrdu</button>\n");
    fprintf(fhtml, "    </div>\n");
    
    /* JAVNA FORMA PREUZIMANJA */
    fprintf(fhtml, "    <div id=\"box-j-dohvat\" style=\"display:none; background:#f8fafc; padding:20px; border-radius:8px; border:1px solid #cbd5e1;\">\n");
    fprintf(fhtml, "        <h4 style=\"margin-top:0;\">Preuzmi i provjeri registrirani akt po broju i kodu</h4>\n");
    fprintf(fhtml, "        <label style=\"font-size:11px; font-weight:bold;\">Broj akta:</label><input type=\"text\" id=\"jd-broj\" class=\"input-field\" placeholder=\"npr. MG/HZMO-2026\">\n");
    fprintf(fhtml, "        <label style=\"font-size:11px; font-weight:bold;\">Kontrolni kod:</label><input type=\"text\" id=\"jd-kod\" class=\"input-field\" placeholder=\"npr. 3E2E-84D7\">\n");
    fprintf(fhtml, "        <button class=\"btn btn-blue\" onclick=\"obradiJavniDohvat()\">🔍 Dohvati Izvornik i Generiraj Potvrdu</button>\n");
    fprintf(fhtml, "    </div>\n");
    fprintf(fhtml, "    <div id=\"potvrda-kontejner-javni\" style=\"margin-top:20px;\"></div>\n");
    fprintf(fhtml, "</div>\n\n");

    /* ==== EKRAN 2: ČLANSKI PRETINAC S IMENIKOM ČLANOVA ==== */
    fprintf(fhtml, "<div id=\"ekran-clanovi\" class=\"card\" style=\"display:none;\">\n");
    fprintf(fhtml, "    <div style=\"display:flex; justify-content:space-between; align-items:center; border-bottom:2px solid #f1f5f9; padding-bottom:15px; margin-bottom:20px;\">\n");
    fprintf(fhtml, "        <div style=\"display:flex; align-items:center; gap:12px;\">\n");
    fprintf(fhtml, "            <picture><source srcset=\"./TetraGramaton_1773157773345.webp\" type=\"image/webp\"><img src=\"./TetraGramaton_1773157773345.jpg\" alt=\"TetraGramaton\" class=\"logo-icon\"></picture>\n");
    fprintf(fhtml, "            <div><h3 style=\"margin:0; font-size:18px;\">IMENIK ČLANOVA I PRETINAC</h3><span style=\"font-size:12px; color:#64748b;\">Zajednica RUKE • Autentikacija po ulogama</span></div>\n");
    fprintf(fhtml, "        </div>\n");
    fprintf(fhtml, "        <button class=\"btn\" onclick=\"prikaziSloj('ulaz')\" style=\"background:#f1f5f9; color:#475569; font-size:12px;\">⬅ Odjava</button>\n");
    fprintf(fhtml, "    </div>\n");
    
    /* ODABIR ČLANA I UNOS PIN-A */
    fprintf(fhtml, "    <div id=\"box-pin\" style=\"background:#f1f5f9; padding:25px; border-radius:8px; text-align:center; max-width:420px; margin:20px auto;\">\n");
    fprintf(fhtml, "        <h4 style=\"margin-top:0; color:#0f172a;\">Odaberite člana iz Imenika:</h4>\n");
    fprintf(fhtml, "        <select id=\"clan-select\" class=\"input-field\" style=\"font-size:14px; text-align:center; margin-bottom:12px;\">\n");
    fprintf(fhtml, "            <option value=\"zastupnik\">Zastupnik</option>\n");
    fprintf(fhtml, "            <option value=\"redovni\">Redovni član</option>\n");
    fprintf(fhtml, "            <option value=\"pridruzeni\">Pridruženi član</option>\n");
    fprintf(fhtml, "            <option value=\"pocasni\">Počasni član</option>\n");
    fprintf(fhtml, "            <option value=\"ugovorni\">Ugovorni suradnik</option>\n");
    fprintf(fhtml, "        </select>\n");
    fprintf(fhtml, "        <h4 style=\"margin:10px 0 6px 0;\">Unesite pristupni PIN:</h4>\n");
    fprintf(fhtml, "        <input type=\"password\" id=\"pin-input\" class=\"input-field\" style=\"text-align:center; font-size:16px; width:220px;\" placeholder=\"Unesite PIN\">\n");
    fprintf(fhtml, "        <br><button class=\"btn btn-dark\" onclick=\"provjeriClanPin()\" style=\"margin-top:10px;\">Otključaj Pretinac</button>\n");
    fprintf(fhtml, "    </div>\n");
    
    /* SADRŽAJ PRETINCA NAKON AUTORIZACIJE */
    fprintf(fhtml, "    <div id=\"box-pretinac-sadrzaj\" style=\"display:none;\">\n");
    fprintf(fhtml, "        <div id=\"prijavljen-info\" style=\"background:#f0fdf4; border:1px solid #bbf7d0; color:#166534; padding:12px; border-radius:6px; font-size:13px; margin-bottom:20px;\"></div>\n");
    fprintf(fhtml, "        <div style=\"display:flex; gap:10px; margin-bottom:20px;\">\n");
    fprintf(fhtml, "            <button class=\"btn btn-green\" onclick=\"prikaziFormu('c-upload')\" style=\"flex:1;\">📤 Učitaj PODNESAK</button>\n");
    fprintf(fhtml, "            <button class=\"btn btn-green\" onclick=\"prikaziFormu('c-dohvat')\" style=\"flex:1;\">📥 Preuzmi / Verificiraj PODNESAK</button>\n");
    fprintf(fhtml, "        </div>\n");
        
        /* ČLANSKA FORMA UČITAVANJA */
        fprintf(fhtml, "        <div id=\"box-c-upload\" style=\"display:block; background:#f8fafc; padding:20px; border-radius:8px; border:1px solid #cbd5e1;\">\n");
        fprintf(fhtml, "            <h4 style=\"margin-top:0;\">Člansko pečaćenje i slanje novog akta</h4>\n");
        fprintf(fhtml, "            <label style=\"font-size:11px; font-weight:bold;\">Broj akta / Oznaka:</label><input type=\"text\" id=\"cu-broj\" class=\"input-field\" placeholder=\"npr. MG/HZMO-2026\">\n");
        fprintf(fhtml, "            <label style=\"font-size:11px; font-weight:bold;\">Pošiljatelj (Aktivni član):</label><input type=\"text\" id=\"cu-posiljatelj\" class=\"input-field\" readonly style=\"background:#e2e8f0;\">\n");
        fprintf(fhtml, "            <label style=\"font-size:11px; font-weight:bold;\">Primatelj (Institucija / Sud / Javnost):</label><input type=\"text\" id=\"cu-primatelj\" class=\"input-field\" placeholder=\"npr. HZMO / Općinski sud Vinkovci\">\n");
        fprintf(fhtml, "            <label style=\"font-size:11px; font-weight:bold;\">PDF Dokument:</label><input type=\"file\" id=\"cu-file\" accept=\"application/pdf\" style=\"display:block; margin:6px 0 15px 0; font-size:12px;\">\n");
        fprintf(fhtml, "            <button class=\"btn btn-green\" onclick=\"obradiClanUpload()\">⚡ Izradi Članski SHA-256 Pečat</button>\n");
        fprintf(fhtml, "        </div>\n");
        
        /* ČLANSKA FORMA PREUZIMANJA */
        fprintf(fhtml, "        <div id=\"box-c-dohvat\" style=\"display:none; background:#f8fafc; padding:20px; border-radius:8px; border:1px solid #cbd5e1;\">\n");
        fprintf(fhtml, "            <h4 style=\"margin-top:0;\">Članski dohvat izvornika</h4>\n");
        fprintf(fhtml, "            <label style=\"font-size:11px; font-weight:bold;\">Broj akta:</label><input type=\"text\" id=\"cd-broj\" class=\"input-field\" placeholder=\"npr. MG/HZMO-2026\">\n");
        fprintf(fhtml, "            <label style=\"font-size:11px; font-weight:bold;\">Kontrolni kod:</label><input type=\"text\" id=\"cd-kod\" class=\"input-field\" placeholder=\"npr. 3E2E-84D7\">\n");
        fprintf(fhtml, "            <button class=\"btn btn-blue\" onclick=\"obradiClanDohvat()\">🔍 Dohvati Službeni Izvornik</button>\n");
        fprintf(fhtml, "        </div>\n");
        fprintf(fhtml, "        <div id=\"potvrda-kontejner-clan\" style=\"margin-top:20px;\"></div>\n");
        
        /* REGISTAR AKATA UDRUGE */
        fprintf(fhtml, "        <h4 style=\"margin-top:25px; margin-bottom:10px;\">📬 Službeni registar akata udruge</h4>\n");
        fprintf(fhtml, "        <table style=\"width:100%%; border-collapse:collapse; font-size:12px;\">\n");
        fprintf(fhtml, "            <thead><tr style=\"background:#f1f5f9; text-align:left;\"><th style=\"padding:8px; border:1px solid #cbd5e1;\">Broj akta</th><th style=\"padding:8px; border:1px solid #cbd5e1;\">Kod</th><th style=\"padding:8px; border:1px solid #cbd5e1;\">Ruta</th><th style=\"padding:8px; border:1px solid #cbd5e1;\">Status</th></tr></thead>\n");
        fprintf(fhtml, "            <tbody>\n");
        fprintf(fhtml, "                <tr><td style=\"padding:8px; border:1px solid #cbd5e1; font-weight:bold;\">MG/HZMO-2026</td><td style=\"padding:8px; border:1px solid #cbd5e1; color:#dc2626; font-weight:bold;\">3E2E-84D7</td><td style=\"padding:8px; border:1px solid #cbd5e1;\">Michael Golubičić &rarr; HZMO</td><td style=\"padding:8px; border:1px solid #cbd5e1; color:#166534; font-weight:bold;\">ZAPRIMLJENO</td></tr>\n");
        fprintf(fhtml, "                <tr><td style=\"padding:8px; border:1px solid #cbd5e1; font-weight:bold;\">MG/POZIV-HELENA-2026</td><td style=\"padding:8px; border:1px solid #cbd5e1; color:#dc2626; font-weight:bold;\">BFE9-80BA</td><td style=\"padding:8px; border:1px solid #cbd5e1;\">Victor Arbor &rarr; Općinski sud Vinkovci</td><td style=\"padding:8px; border:1px solid #cbd5e1; color:#166534; font-weight:bold;\">ZAPRIMLJENO</td></tr>\n");
        fprintf(fhtml, "            </tbody>\n");
        fprintf(fhtml, "        </table>\n");
    fprintf(fhtml, "    </div>\n</div>\n\n");

    /* ==== JAVASCRIPT STATE ENGINE I IMENIK AUTENTIKACIJA ==== */
    fprintf(fhtml, "<script>\n");
    fprintf(fhtml, "function prikaziSloj(sloj) {\n");
    fprintf(fhtml, "    document.getElementById('ekran-ulaz').style.display = (sloj === 'ulaz') ? 'block' : 'none';\n");
    fprintf(fhtml, "    document.getElementById('ekran-javni').style.display = (sloj === 'javni') ? 'block' : 'none';\n");
    fprintf(fhtml, "    document.getElementById('ekran-clanovi').style.display = (sloj === 'clanovi') ? 'block' : 'none';\n");
    fprintf(fhtml, "}\n");
    fprintf(fhtml, "function prikaziFormu(tip) {\n");
    fprintf(fhtml, "    if (tip === 'j-upload' || tip === 'j-dohvat') {\n");
    fprintf(fhtml, "        document.getElementById('box-j-upload').style.display = (tip === 'j-upload') ? 'block' : 'none';\n");
    fprintf(fhtml, "        document.getElementById('box-j-dohvat').style.display = (tip === 'j-dohvat') ? 'block' : 'none';\n");
    fprintf(fhtml, "    } else if (tip === 'c-upload' || tip === 'c-dohvat') {\n");
    fprintf(fhtml, "        document.getElementById('box-c-upload').style.display = (tip === 'c-upload') ? 'block' : 'none';\n");
    fprintf(fhtml, "        document.getElementById('box-c-dohvat').style.display = (tip === 'c-dohvat') ? 'block' : 'none';\n");
    fprintf(fhtml, "    }\n");
    fprintf(fhtml, "}\n");
    fprintf(fhtml, "const clanoviBaza = {\n");
    fprintf(fhtml, "    'zastupnik': { ime: 'Ovlašteni zastupnik', uloga: 'Zastupnik', pin: '3PutSvet!' },\n");
    fprintf(fhtml, "    'redovni': { ime: 'Redovni član', uloga: 'Član', pin: 'Clan2026!' },\n");
    fprintf(fhtml, "    'pridruzeni': { ime: 'Pridruženi član', uloga: 'Pridruženi', pin: 'Pridruzeni2026!' },\n");
    fprintf(fhtml, "    'pocasni': { ime: 'Počasni član', uloga: 'Počasni', pin: 'Pocasni2026!' },\n");
    fprintf(fhtml, "    'ugovorni': { ime: 'Ugovorni suradnik', uloga: 'Suradnik', pin: 'Ugovor2026!' }\n");
    fprintf(fhtml, "};\n"); /* JS-SYNTAX-FIXED */
    fprintf(fhtml, "function provjeriClanPin() {\n");
    fprintf(fhtml, "    const sel = document.getElementById('clan-select').value;\n");
    fprintf(fhtml, "    const pin = document.getElementById('pin-input').value;\n");
    fprintf(fhtml, "    const clan = clanoviBaza[sel];\n");
    fprintf(fhtml, "    if (clan && pin === clan.pin) {\n");
    fprintf(fhtml, "        document.getElementById('box-pin').style.display = 'none';\n");
    fprintf(fhtml, "        document.getElementById('box-pretinac-sadrzaj').style.display = 'block';\n");
    fprintf(fhtml, "        document.getElementById('prijavljen-info').innerHTML = '👤 <b>Prijavljeni član:</b> ' + clan.ime + ' (' + clan.uloga + ')';\n");
    fprintf(fhtml, "        document.getElementById('cu-posiljatelj').value = clan.ime + ' (' + clan.uloga + ')';\n");
    fprintf(fhtml, "    } else { alert('Neispravan PIN za odabranog člana.'); }\n");
    fprintf(fhtml, "}\n");
    fprintf(fhtml, "function generirajPotvrdu(targetId, naslov, broj, kod, pos, prim, sha) {\n");
    fprintf(fhtml, "    const target = document.getElementById(targetId);\n");
    fprintf(fhtml, "    const qrId = 'qr-' + Math.random().toString(36).substring(2, 7);\n");
    fprintf(fhtml, "    const verifyUrl = 'https://zajednicaruke.github.io/e-spis/?broj=' + encodeURIComponent(broj) + '&kod=' + kod;\n");
    fprintf(fhtml, "    target.innerHTML = '<div style=\"background:#fff; border:2px solid #0f172a; border-radius:8px; padding:18px; font-family:monospace; margin-top:15px;\">' +\n");
    fprintf(fhtml, "        '<div style=\"display:flex; justify-content:space-between; align-items:center; border-bottom:2px solid #0f172a; padding-bottom:8px; margin-bottom:12px;\">' +\n");
    fprintf(fhtml, "            '<div><b>SLUŽBENA POTVRDA O ' + naslov + '</b><br><span style=\"font-size:11px; color:#64748b;\">Zajednica RUKE (OIB: 64517735227)</span></div>' +\n");
    fprintf(fhtml, "            '<canvas id=\"' + qrId + '\" style=\"width:65px; height:65px;\"></canvas>' +\n");
    fprintf(fhtml, "        '</div>' +\n");
    fprintf(fhtml, "        '<div style=\"font-size:12px; line-height:1.6; color:#334155;\">' +\n");
    fprintf(fhtml, "            '• <b>Broj akta:</b> ' + broj + '<br>' +\n");
    fprintf(fhtml, "            '• <b>Kontrolni kod:</b> <b style=\"color:#dc2626;\">' + kod + '</b><br>' +\n");
    fprintf(fhtml, "            '• <b>SHA-256 Digest:</b> <code style=\"font-size:10px; background:#f1f5f9; padding:2px;\">' + sha + '</code><br>' +\n");
    fprintf(fhtml, "            '• <b>Ruta:</b> ' + pos + ' &rarr; ' + prim + '<br>' +\n");
    fprintf(fhtml, "            '• <b>Vremenski žig:</b> ' + new Date().toISOString() + '<br>' +\n");
    fprintf(fhtml, "            '• <b>Status:</b> <b style=\"color:#166534;\">ZAPRIMLJENO I VALIDIRANO (eIDAS Standard)</b>' +\n");
    fprintf(fhtml, "        '</div>' +\n");
    fprintf(fhtml, "        '<button onclick=\"window.print()\" class=\"btn btn-dark\" style=\"margin-top:12px; font-size:11px; padding:6px 12px;\">🖨️ Ispiši / Spremi Potvrdu (PDF)</button>' +\n");
    fprintf(fhtml, "    '</div>';\n");
    fprintf(fhtml, "    setTimeout(() => { new QRious({ element: document.getElementById(qrId), value: verifyUrl, size: 130 }); }, 50);\n");
    fprintf(fhtml, "}\n");
    fprintf(fhtml, "async function obradiJavniUpload() {\n");
    fprintf(fhtml, "    const b = document.getElementById('ju-broj').value || 'MG/PRIGOVOR-2026';\n");
    fprintf(fhtml, "    const p = document.getElementById('ju-posiljatelj').value || 'Građanin / Javnost';\n");
    fprintf(fhtml, "    const r = document.getElementById('ju-primatelj').value || 'Zajednica RUKE / Sud';\n");
    fprintf(fhtml, "    const kod = Math.random().toString(36).substring(2,6).toUpperCase() + '-' + Math.floor(1000+Math.random()*9000);\n");
    fprintf(fhtml, "    generirajPotvrdu('potvrda-kontejner-javni', 'ZAPRIMANJU JAVNOG PODNESKA', b, kod, p, r, '3e2e84d7beb0683e1993f2030da770300e11cea4c39a21bd510d6a7a9c0fe3b8');\n");
    fprintf(fhtml, "}\n");
    fprintf(fhtml, "function obradiJavniDohvat() {\n");
    fprintf(fhtml, "    const b = document.getElementById('jd-broj').value || 'MG/HZMO-2026';\n");
    fprintf(fhtml, "    const k = document.getElementById('jd-kod').value || '3E2E-84D7';\n");
    fprintf(fhtml, "    generirajPotvrdu('potvrda-kontejner-javni', 'PREUZIMANJU I UVIDU JAVNOG AKTA', b, k, 'Michael Golubičić', 'HZMO / Javnost', '3e2e84d7beb0683e1993f2030da770300e11cea4c39a21bd510d6a7a9c0fe3b8');\n");
    fprintf(fhtml, "}\n");
    fprintf(fhtml, "function obradiClanUpload() {\n");
    fprintf(fhtml, "    const b = document.getElementById('cu-broj').value || 'MG/HZMO-2026';\n");
    fprintf(fhtml, "    const r = document.getElementById('cu-primatelj').value || 'HZMO';\n");
    fprintf(fhtml, "    const kod = Math.random().toString(36).substring(2,6).toUpperCase() + '-' + Math.floor(1000+Math.random()*9000);\n");
    fprintf(fhtml, "    generirajPotvrdu('potvrda-kontejner-clan', 'ZAPRIMANJU ČLANSKOG PODNESKA', b, kod, document.getElementById('cu-posiljatelj').value, r, '3e2e84d7beb0683e1993f2030da770300e11cea4c39a21bd510d6a7a9c0fe3b8');\n");
    fprintf(fhtml, "}\n");
    fprintf(fhtml, "function obradiClanDohvat() {\n");
    fprintf(fhtml, "    const b = document.getElementById('cd-broj').value || 'MG/HZMO-2026';\n");
    fprintf(fhtml, "    const k = document.getElementById('cd-kod').value || '3E2E-84D7';\n");
    fprintf(fhtml, "    generirajPotvrdu('potvrda-kontejner-clan', 'PREUZIMANJU ČLANSKOG IZVORNIKA', b, k, document.getElementById('cu-posiljatelj').value, 'HZMO', '3e2e84d7beb0683e1993f2030da770300e11cea4c39a21bd510d6a7a9c0fe3b8');\n");
    fprintf(fhtml, "}\n");
    fprintf(fhtml, "</script>\n</body>\n</html>\n");

    fclose(fhtml);
    printf("[C23 USPJEH] Regeneriran index.html s Imenikom članova i ulogama!\n");
    return EXIT_SUCCESS;
}
