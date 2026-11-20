(function () {
  const QES_BASE = "https://ydeswffyfomboflcvnfm.supabase.co/storage/v1/object/public/QES-PDF";
  const MANIFEST_URL = "./manifest_oglasna_ploca.json";

  let ucitaniIme = "";
  let ucitaniSha = "";
  let zadnjiArtefakt = null;
  let zadnjiPdfUrl = "";
  let oglasneStavke = [];

  async function sha256Hex(buf) {
    const d = await crypto.subtle.digest("SHA-256", buf);
    return Array.from(new Uint8Array(d)).map((b) => b.toString(16).padStart(2, "0")).join("");
  }

  function aktivniSha() {
    if (/^[0-9a-f]{64}$/.test(ucitaniSha)) return ucitaniSha;
    const q = new URLSearchParams(window.location.search);
    const u = (q.get("sha256") || "").trim().toLowerCase();
    if (/^[0-9a-f]{64}$/.test(u)) return u;
    for (const el of document.querySelectorAll("input, textarea, select")) {
      const m = ((el.value || "").trim().toLowerCase()).match(/[0-9a-f]{64}/);
      if (m) return m[0];
    }
    if (document.body) {
      const m2 = document.body.innerText.toLowerCase().match(/[0-9a-f]{64}/);
      if (m2) return m2[0];
    }
    return "";
  }

  function osigurajPdfTipku(btnPng) {
    let btnPdf = document.getElementById("btn-qr-pdf-dl");
    if (!btnPdf) {
      btnPdf = document.createElement("button");
      btnPdf.type = "button";
      btnPdf.id = "btn-qr-pdf-dl";
      btnPdf.textContent = "Preuzmi QR (.PDF)";
      btnPdf.disabled = true;
      if (btnPng && btnPng.parentElement) {
        btnPng.insertAdjacentElement("afterend", btnPdf);
      }
    }
    return btnPdf;
  }

  window.preuzmiQrPdfPodneska = function () {
    if (!zadnjiArtefakt || !zadnjiPdfUrl) window.generirajQrNaPodnesak();
    if (!zadnjiArtefakt || !zadnjiPdfUrl) return;
    const a = document.createElement("a");
    a.href = zadnjiPdfUrl;
    a.download = zadnjiArtefakt.pdfIme;
    document.body.appendChild(a);
    a.click();
    a.remove();
  };

  window.generirajQrNaPodnesak = function () {
    const sha = aktivniSha();
    if (!/^[0-9a-f]{64}$/.test(sha)) {
      alert("Nema aktivnog 64-znakovnog SHA-256 za Podnesak.");
      return;
    }
    if (!window.RukeQrZapis) return;
    if (zadnjiPdfUrl) URL.revokeObjectURL(zadnjiPdfUrl);
    const naziv = ucitaniIme || `${sha.slice(0, 12)}_podnesak.pdf`;
    zadnjiArtefakt = window.RukeQrZapis.pripremiArtefakte(sha, naziv);
    zadnjiPdfUrl = zadnjiArtefakt.pdfBlob ? URL.createObjectURL(zadnjiArtefakt.pdfBlob) : "";

    const btnDl = document.getElementById("btn-qr-dl");
    if (btnDl) btnDl.disabled = false;
    const btnPdf = osigurajPdfTipku(btnDl);
    if (btnPdf) btnPdf.disabled = !zadnjiPdfUrl;

    let wrap = document.getElementById("ruke-qr-v15-box");
    if (!wrap) {
      wrap = document.createElement("div");
      wrap.id = "ruke-qr-v15-box";
      wrap.style.marginTop = "10px"; wrap.style.padding = "10px";
      wrap.style.border = "1px solid #777"; wrap.style.background = "#fff"; wrap.style.color = "#111";
      const anchor = (btnPdf && btnPdf.parentElement) ? btnPdf.parentElement : (btnDl ? btnDl.parentElement : document.body);
      anchor.appendChild(wrap);
    }
    wrap.innerHTML =
      `<div><b>Broj zapisa:</b> ${zadnjiArtefakt.brojZapisa} | <b>Kontrolni broj:</b> ${zadnjiArtefakt.kod}</div>` +
      `<div style="margin-top:4px"><b>QR URI:</b> <a href="${zadnjiArtefakt.qrUri}" target="_blank" rel="noopener">${zadnjiArtefakt.qrUri}</a></div>` +
      `<div style="margin-top:6px"><b>QR kod</b></div>` +
      `<div style="margin-top:6px"><img src="${zadnjiArtefakt.pngDataUrl}" alt="QR kod" width="190" height="190" style="border:1px solid #ccc;padding:4px;background:#fff"></div>` +
      `<div style="margin-top:8px">` +
      `<div>1. QR kod kao PNG (${zadnjiArtefakt.pngIme})</div>` +
      `<div>2. PDF s QR kodom, brojem zapisa i kontrolnim brojem (${zadnjiArtefakt.pdfIme})</div>` +
      `</div>`;
  };

  window.preuzmiQrPodneska = function () {
    if (!zadnjiArtefakt) window.generirajQrNaPodnesak();
    if (!zadnjiArtefakt) return;
    const a = document.createElement("a");
    a.href = zadnjiArtefakt.pngDataUrl; a.download = zadnjiArtefakt.pngIme;
    document.body.appendChild(a); a.click(); a.remove();
  };

  async function preuzmiIzSupabase(sha) {
    const p12 = sha.slice(0, 12);
    const urls = [
      `${QES_BASE}/${p12}_podnesak.pdf?t=${Date.now()}`,
      `${QES_BASE}/${sha}.pdf?t=${Date.now()}`
    ];
    for (const u of urls) {
      try {
        const r = await fetch(u, { cache: "no-store" });
        if (!r.ok) continue;
        const buf = await r.arrayBuffer();
        const calc = await sha256Hex(buf);
        if (calc !== sha) continue;
        const blob = new Blob([buf], { type: "application/pdf" });
        return { blobUrl: URL.createObjectURL(blob), fname: `${p12}_podnesak.pdf` };
      } catch (_) {}
    }
    return null;
  }

  async function ucitajManifestOglasne() {
    try {
      const r = await fetch(`${MANIFEST_URL}?t=${Date.now()}`, { cache: "no-store" });
      if (!r.ok) return null;
      const data = await r.json();
      if (data && Array.isArray(data.stavke)) {
        oglasneStavke = data.stavke.filter((s) => /^[0-9a-f]{64}$/i.test((s && s.sha256) || ""));
        return data;
      }
    } catch (_) {}
    return null;
  }

  function pronadiOglasnaSelect() {
    for (const sel of document.querySelectorAll("select")) {
      const txt = (sel.innerText || sel.textContent || "").toLowerCase();
      if (txt.includes("oglasna plo") || txt.includes("odaberite akt") || sel.options.length > 0) {
        return sel;
      }
    }
    return document.querySelector("select");
  }

  async function primijeniStavkuOglasne(stavka, sel) {
    if (!stavka || !/^[0-9a-f]{64}$/i.test(stavka.sha256)) return;
    const sha = stavka.sha256.toLowerCase();
    ucitaniSha = sha;
    ucitaniIme = `${sha.slice(0, 12)}_podnesak.pdf`;

    const scope = (sel && sel.closest("section, article, div")) || document;
    const inputs = Array.from(scope.querySelectorAll("input[type='text'], input:not([type])"));
    if (inputs[0]) inputs[0].value = stavka.kontrolni || `[${sha.slice(0, 4)}-${sha.slice(4, 8)}]`.toUpperCase();
    if (inputs[1]) inputs[1].value = stavka.broj_zapisa || `RUKE-${sha.slice(0, 12).toUpperCase()}`;

    window.generirajQrNaPodnesak();

    const res = await preuzmiIzSupabase(sha);
    if (res) {
      let dlBox = document.getElementById("ruke-qr-izravno-pdf");
      if (!dlBox) {
        dlBox = document.createElement("div");
        dlBox.id = "ruke-qr-izravno-pdf";
        dlBox.style.marginTop = "10px"; dlBox.style.padding = "8px";
        dlBox.style.border = "1px solid #2da44e"; dlBox.style.background = "#fff"; dlBox.style.color = "#111";
        if (sel && sel.parentElement) sel.parentElement.appendChild(dlBox);
        else document.body.appendChild(dlBox);
      }
      dlBox.innerHTML =
        `<div><b>Oglasna ploča (${stavka.oznaka || "e-Spis"}):</b> ${stavka.naslov || res.fname}</div>` +
        `<div style="margin-top:4px"><b>Netaknuti izvornik (SHA-256 potvrđen):</b> <a href="${res.blobUrl}" download="${res.fname}">${res.fname}</a></div>`;
    }
  }

  async function inicijalizirajOglasnuPlocu() {
    const sel = pronadiOglasnaSelect();
    if (!sel) return;
    await ucitajManifestOglasne();

    const existingShas = new Set(Array.from(sel.options).map((o) => (o.value || "").toLowerCase()));
    if (sel.options[0]) {
      sel.options[0].textContent = oglasneStavke.length > 0
        ? `Oglasna ploča (${oglasneStavke.length})`
        : "Oglasna ploča";
    }

    for (const st of oglasneStavke) {
      const sha = st.sha256.toLowerCase();
      if (existingShas.has(sha)) continue;
      const opt = document.createElement("option");
      opt.value = sha;
      opt.textContent = `[${st.kontrolni || sha.slice(0, 4).toUpperCase()}] ${st.oznaka || ""} — ${st.naslov || sha.slice(0, 12)}`.trim();
      sel.appendChild(opt);
      existingShas.add(sha);
    }

    sel.addEventListener("change", async function () {
      const val = (sel.value || "").trim().toLowerCase();
      const hit = oglasneStavke.find((s) => s.sha256.toLowerCase() === val);
      if (hit) await primijeniStavkuOglasne(hit, sel);
    });
  }

  document.addEventListener("change", async function (ev) {
    const inp = ev.target;
    if (!inp || inp.type !== "file" || !inp.files || !inp.files[0]) return;
    const f = inp.files[0];
    const buf = await f.arrayBuffer();
    ucitaniIme = f.name || "podnesak.pdf";
    ucitaniSha = await sha256Hex(buf);
  }, true);

  document.addEventListener("click", async function (ev) {
    const t = ev.target ? ev.target.closest("button, a") : null;
    if (!t) return;
    const lbl = ((t.innerText || t.textContent || "")).trim().toLowerCase();
    if (lbl.includes("generiraj qr")) {
      ev.preventDefault(); ev.stopPropagation(); ev.stopImmediatePropagation();
      window.generirajQrNaPodnesak();
    } else if (lbl.includes("preuzmi qr (.pdf)") || lbl.includes("preuzmi qr pdf")) {
      ev.preventDefault(); ev.stopPropagation(); ev.stopImmediatePropagation();
      window.preuzmiQrPdfPodneska();
    } else if (lbl.includes("preuzmi qr")) {
      ev.preventDefault(); ev.stopPropagation(); ev.stopImmediatePropagation();
      window.preuzmiQrPodneska();
    } else if (lbl.includes("dohvati izvornik") || lbl.includes("preuzmi izvornik")) {
      const sha = aktivniSha();
      if (/^[0-9a-f]{64}$/.test(sha)) {
        ev.preventDefault(); ev.stopPropagation(); ev.stopImmediatePropagation();
        const res = await preuzmiIzSupabase(sha);
        if (!res) { alert("Izvornik nije pronađen u QES-PDF bucketu."); return; }
        const a = document.createElement("a");
        a.href = res.blobUrl; a.download = res.fname;
        document.body.appendChild(a); a.click(); a.remove();
        setTimeout(() => URL.revokeObjectURL(res.blobUrl), 15000);
      }
    }
  }, true);


  function osigurajKontekstTraku() {
    let bar = document.getElementById("ruke-aktivni-kontekst");
    if (bar) return bar;
    bar = document.createElement("div");
    bar.id = "ruke-aktivni-kontekst";
    for (const h of document.querySelectorAll("h1,h2,h3,h4,div,section")) {
      const tx = (h.innerText || h.textContent || "");
      if (tx.includes("1) Ucitavanje Podneska") || tx.includes("1) Učitavanje Podneska")) {
        const host = h.closest("section, article, div") || h;
        if (host && host.parentElement) {
          host.parentElement.insertBefore(bar, host);
          return bar;
        }
      }
    }
    document.body.insertBefore(bar, document.body.firstChild);
    return bar;
  }

  function osvjeziKontekstTraku(statusIzvornika) {
    const bar = osigurajKontekstTraku();
    const sha = aktivniSha();
    const kod = sha ? `[${sha.slice(0, 4)}-${sha.slice(4, 8)}]`.toUpperCase() : "—";
    const broj = sha ? `RUKE-${sha.slice(0, 12).toUpperCase()}` : "—";
    const shaPrikaz = sha ? `${sha.slice(0, 16)}…${sha.slice(-8)}` : "Nije aktivan";
    const st = statusIzvornika || (sha ? "Spremno za SHA-256 / QES-PDF provjeru" : "Odaberite stavku na Oglasnoj ploči ili učitajte podnesak");
    bar.innerHTML =
      `<b>Kontekst podneska:</b> Kontrolni: <code>${kod}</code> | Broj zapisa: <code>${broj}</code><br>` +
      `<b>SHA-256:</b> <code>${shaPrikaz}</code> | <b>Status:</b> ${st}`;

    const btnVal = document.getElementById("ruke-validiraj-ucitani-btn");
    if (btnVal) btnVal.disabled = !sha;
  }

  function posloziKompaktneTipke() {
    const btnGen = document.getElementById("btn-qr-gen");
    const btnPng = document.getElementById("btn-qr-dl");
    const btnPdf = osigurajPdfTipku(btnPng);
    const first = btnGen || btnPng || btnPdf;
    if (first && first.parentElement && !first.parentElement.classList.contains("ruke-qr-btn-red")) {
      const row = document.createElement("div");
      row.className = "ruke-qr-btn-red";
      first.parentElement.insertBefore(row, first);
      if (btnGen) row.appendChild(btnGen);
      if (btnPng) row.appendChild(btnPng);
      if (btnPdf) row.appendChild(btnPdf);
    }

    if (!document.getElementById("ruke-validiraj-ucitani-btn")) {
      for (const el of document.querySelectorAll("h2,h3,h4,label,div")) {
        const tx = (el.innerText || el.textContent || "").trim().toLowerCase();
        if (tx.includes("4) dss validacija") || tx.includes("dss validacija")) {
          const box = el.closest("section, article, div") || el.parentElement;
          if (box && !box.querySelector("#ruke-validiraj-ucitani-btn")) {
            const b = document.createElement("button");
            b.type = "button";
            b.id = "ruke-validiraj-ucitani-btn";
            b.textContent = "Validiraj učitani podnesak";
            b.disabled = !aktivniSha();
            b.addEventListener("click", function () {
              const s = aktivniSha();
              if (!s) return;
              osvjeziKontekstTraku(`DSS/SHA-256 kontekst potvrđen (${s.slice(0, 12)}…)`);
            });
            box.appendChild(b);
            break;
          }
        }
      }
    }
  }

  window.addEventListener("DOMContentLoaded", async function () {
    posloziKompaktneTipke();
    osvjeziKontekstTraku("");
    await inicijalizirajOglasnuPlocu();
    osvjeziKontekstTraku("");

    const sha = aktivniSha();
    if (/^[0-9a-f]{64}$/.test(sha)) {
      window.generirajQrNaPodnesak();
      osvjeziKontekstTraku("Provjera netaknutog izvornika u QES-PDF...");
      const res = await preuzmiIzSupabase(sha);
      if (res) {
        osvjeziKontekstTraku(`Netaknuti izvornik spreman: ${res.fname}`);
        let dlBox = document.getElementById("ruke-qr-izravno-pdf");
        if (!dlBox) {
          dlBox = document.createElement("div");
          dlBox.id = "ruke-qr-izravno-pdf";
          dlBox.style.marginTop = "10px"; dlBox.style.padding = "8px";
          dlBox.style.border = "1px solid #2da44e"; dlBox.style.background = "#fff"; dlBox.style.color = "#111";
          document.body.appendChild(dlBox);
        }
        dlBox.innerHTML = `<b>Netaknuti izvornik spreman za preuzimanje:</b> <a href="${res.blobUrl}" download="${res.fname}">${res.fname}</a>`;
      } else {
        osvjeziKontekstTraku("Izvornik nije pronađen u QES-PDF bucketu");
      }
    }
  });
})();
