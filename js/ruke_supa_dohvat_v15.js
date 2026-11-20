(function () {
  const QES_BASE = "https://ydeswffyfomboflcvnfm.supabase.co/storage/v1/object/public/QES-PDF";

  let ucitaniIme = "";
  let ucitaniSha = "";
  let zadnjiArtefakt = null;
  let zadnjiPdfUrl = "";

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
    zadnjiArtefakt = window.RukeQrZapis.pripremiArtefakte(sha, ucitaniIme || `${sha.slice(0, 12)}_podnesak.pdf`);
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
    const urls = [`${QES_BASE}/${p12}_podnesak.pdf?t=${Date.now()}`, `${QES_BASE}/${sha}.pdf?t=${Date.now()}`];
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
    } else if (lbl.includes("preuzmi izvornik")) {
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

 
window.addEventListener("DOMContentLoaded", async function () {
    const btnDl = document.getElementById("btn-qr-dl");
    osigurajPdfTipku(btnDl);
    const sha = aktivniSha();
    if (/^[0-9a-f]{64}$/.test(sha)) {
      window.generirajQrNaPodnesak();
      const res = await preuzmiIzSupabase(sha);
      if (res) {
        let dlBox = document.getElementById("ruke-qr-izravno-pdf");
        if (!dlBox) {
          dlBox = document.createElement("div");
          dlBox.id = "ruke-qr-izravno-pdf";
          dlBox.style.marginTop = "10px"; dlBox.style.padding = "8px";
          dlBox.style.border = "1px solid #2da44e"; dlBox.style.background = "#fff"; dlBox.style.color = "#111";
          document.body.appendChild(dlBox);
        }
        dlBox.innerHTML = `<b>Netaknuti izvornik spreman za preuzimanje:</b> <a href="${res.blobUrl}" download="${res.fname}">${res.fname}</a>`;
      }
    }
  });
})();
