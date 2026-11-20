(function () {
  const PORTAL_BASE = "https://zajednicaruke.github.io/e-spis/";

  const EXP = new Uint8Array(512);
  const LOG = new Uint8Array(256);
  (function initGF() {
    let x = 1;
    for (let i = 0; i < 255; i++) {
      EXP[i] = x; LOG[x] = i;
      x <<= 1; if (x & 0x100) x ^= 0x11d;
    }
    for (let i = 255; i < 512; i++) EXP[i] = EXP[i - 255];
  })();
  const gfMul = (a, b) => (a === 0 || b === 0) ? 0 : EXP[LOG[a] + LOG[b]];

  function rsGenPoly(ecLen) {
    let poly = [1];
    for (let i = 0; i < ecLen; i++) {
      const next = new Array(poly.length + 1).fill(0);
      for (let j = 0; j < poly.length; j++) {
        next[j] ^= poly[j];
        next[j + 1] ^= gfMul(poly[j], EXP[i]);
      }
      poly = next;
    }
    return poly;
  }

  function rsEncode(data, ecLen) {
    const gen = rsGenPoly(ecLen);
    const ecc = new Array(ecLen).fill(0);
    for (let i = 0; i < data.length; i++) {
      const factor = data[i] ^ ecc[0];
      ecc.shift(); ecc.push(0);
      for (let j = 0; j < ecLen; j++) ecc[j] ^= gfMul(gen[j + 1], factor);
    }
    return ecc;
  }

  const QR_L_SPEC = {
    1:  [26,  7,  1, 19, 0, 0, 0],
    2:  [44,  10, 1, 34, 0, 0, 7],
    3:  [70,  15, 1, 55, 0, 0, 7],
    4:  [100, 20, 1, 80, 0, 0, 7],
    5:  [134, 26, 1, 108, 0, 0, 7],
    6:  [172, 18, 2, 68, 0, 0, 7],
    7:  [196, 20, 2, 78, 0, 0, 0],
    8:  [242, 24, 2, 97, 0, 0, 0],
    9:  [292, 30, 2, 116, 0, 0, 0],
    10: [346, 18, 2, 68, 2, 69, 0]
  };

  function alignCenters(ver) {
    if (ver === 1) return [];
    const dim = 17 + 4 * ver;
    if (ver <= 6) return [6, dim - 7];
    if (ver === 7) return [6, 22, dim - 7];
    if (ver === 8) return [6, 24, dim - 7];
    if (ver === 9) return [6, 26, dim - 7];
    return [6, 28, dim - 7];
  }

  function maskFn(maskId, r, c) {
    switch (maskId) {
      case 0: return (r + c) % 2 === 0;
      case 1: return r % 2 === 0;
      case 2: return c % 3 === 0;
      case 3: return (r + c) % 3 === 0;
      case 4: return (Math.floor(r / 2) + Math.floor(c / 3)) % 2 === 0;
      case 5: return ((r * c) % 2) + ((r * c) % 3) === 0;
      case 6: return ((((r * c) % 2) + ((r * c) % 3)) % 2) === 0;
      default: return ((((r + c) % 2) + ((r * c) % 3)) % 2) === 0;
    }
  }

  function penaltyScore(mat, size) {
    let score = 0;
    for (let r = 0; r < size; r++) {
      let run = 1;
      for (let c = 1; c < size; c++) {
        if (mat[r][c] === mat[r][c - 1]) run++;
        else { if (run >= 5) score += 3 + (run - 5); run = 1; }
      }
      if (run >= 5) score += 3 + (run - 5);
    }
    for (let c = 0; c < size; c++) {
      let run = 1;
      for (let r = 1; r < size; r++) {
        if (mat[r][c] === mat[r - 1][c]) run++;
        else { if (run >= 5) score += 3 + (run - 5); run = 1; }
      }
      if (run >= 5) score += 3 + (run - 5);
    }
    for (let r = 0; r < size - 1; r++) {
      for (let c = 0; c < size - 1; c++) {
        const v = mat[r][c];
        if (v === mat[r][c + 1] && v === mat[r + 1][c] && v === mat[r + 1][c + 1]) score += 3;
      }
    }
    return score;
  }

  function formatBits(ecBits, maskId) {
    const data = ((ecBits & 3) << 3) | (maskId & 7);
    let rem = data << 10;
    const poly = 0b10100110111;
    for (let i = 14; i >= 10; i--) if ((rem >>> i) & 1) rem ^= (poly << (i - 10));
    return ((data << 10) | (rem & 0x3ff)) ^ 0b101010000010010;
  }

  function versionBits(ver) {
    let rem = ver << 12;
    const poly = 0b1111100100101;
    for (let i = 17; i >= 12; i--) if ((rem >>> i) & 1) rem ^= (poly << (i - 12));
    return (ver << 12) | (rem & 0xfff);
  }

  function buildIsoQr(text) {
    const utf8 = new TextEncoder().encode(text);
    const countBits = 8;
    let ver = 0;
    for (let v = 1; v <= 10; v++) {
      const sp = QR_L_SPEC[v];
      const dataCw = sp[2] * sp[3] + sp[4] * sp[5];
      if (4 + countBits + utf8.length * 8 <= dataCw * 8) { ver = v; break; }
    }
    if (!ver) return null;
    const sp = QR_L_SPEC[ver];
    const [, ecPerBlk, b1, cw1, b2, cw2, remBits] = sp;
    const dataCap = b1 * cw1 + b2 * cw2;
    const bits = [];
    const push = (val, len) => { for (let i = len - 1; i >= 0; i--) bits.push((val >>> i) & 1); };
    push(0b0100, 4); push(utf8.length, countBits);
    for (const b of utf8) push(b, 8);
    const term = Math.min(4, dataCap * 8 - bits.length);
    if (term > 0) push(0, term);
    while (bits.length % 8 !== 0) bits.push(0);
    const cw = [];
    for (let i = 0; i < bits.length; i += 8) {
      let v = 0; for (let j = 0; j < 8; j++) v = (v << 1) | bits[i + j];
      cw.push(v);
    }
    const pads = [0xec, 0x11]; let pIdx = 0;
    while (cw.length < dataCap) { cw.push(pads[pIdx % 2]); pIdx++; }
    const dataBlocks = [], ecBlocks = [];
    let pos = 0;
    for (let i = 0; i < b1; i++) { const blk = cw.slice(pos, pos + cw1); pos += cw1; dataBlocks.push(blk); ecBlocks.push(rsEncode(blk, ecPerBlk)); }
    for (let i = 0; i < b2; i++) { const blk = cw.slice(pos, pos + cw2); pos += cw2; dataBlocks.push(blk); ecBlocks.push(rsEncode(blk, ecPerBlk)); }
    const finalCw = [];
    const maxLen = Math.max(cw1, cw2);
    for (let i = 0; i < maxLen; i++) for (const blk of dataBlocks) if (i < blk.length) finalCw.push(blk[i]);
    for (let i = 0; i < ecPerBlk; i++) for (const blk of ecBlocks) if (i < blk.length) finalCw.push(blk[i]);
    const stream = [];
    for (const w of finalCw) for (let i = 7; i >= 0; i--) stream.push((w >>> i) & 1);
    for (let i = 0; i < remBits; i++) stream.push(0);
    const size = 17 + 4 * ver;
    const base = Array.from({ length: size }, () => new Int8Array(size));
    const res = Array.from({ length: size }, () => new Uint8Array(size));
    function setFunc(r, c, d) {
      if (r >= 0 && r < size && c >= 0 && c < size) { base[r][c] = d ? 1 : -1; res[r][c] = 1; }
    }
    function finder(r0, c0) {
      for (let r = -1; r <= 7; r++) for (let c = -1; c <= 7; c++) {
        const rr = r0 + r, cc = c0 + c;
        if (rr < 0 || rr >= size || cc < 0 || cc >= size) continue;
        const box = r >= 0 && r <= 6 && c >= 0 && c <= 6;
        const ring = r === 0 || r === 6 || c === 0 || c === 6;
        const dot = r >= 2 && r <= 4 && c >= 2 && c <= 4;
        setFunc(rr, cc, box ? (ring || dot) : false);
      }
    }
    finder(0, 0); finder(0, size - 7); finder(size - 7, 0);
    const centers = alignCenters(ver);
    for (const r of centers) for (const c of centers) {
      if ((r === 6 && c === 6) || (r === 6 && c === size - 7) || (r === size - 7 && c === 6)) continue;
      for (let dr = -2; dr <= 2; dr++) for (let dc = -2; dc <= 2; dc++) {
        const d = Math.max(Math.abs(dr), Math.abs(dc));
        setFunc(r + dr, c + dc, d === 2 || d === 0);
      }
    }
    for (let i = 8; i < size - 8; i++) { setFunc(6, i, i % 2 === 0); setFunc(i, 6, i % 2 === 0); }
    setFunc(size - 8, 8, true);
    for (let i = 0; i < 9; i++) { if (i !== 6) { res[8][i] = 1; res[i][8] = 1; } }
    for (let i = 0; i < 8; i++) { res[8][size - 1 - i] = 1; res[size - 1 - i][8] = 1; }
    if (ver >= 7) for (let r = 0; r < 6; r++) for (let c = size - 11; c < size - 8; c++) { res[r][c] = 1; res[c][r] = 1; }
    let bIdx = 0, up = true;
    for (let col = size - 1; col >= 1; col -= 2) {
      if (col === 6) col = 5;
      const rows = up ? Array.from({ length: size }, (_, k) => size - 1 - k) : Array.from({ length: size }, (_, k) => k);
      for (const r of rows) for (const dc of [0, -1]) {
        const c = col + dc;
        if (c < 0 || res[r][c]) continue;
        base[r][c] = (bIdx < stream.length && stream[bIdx++]) ? 1 : -1;
      }
      up = !up;
    }
    const hC = [0, 1, 2, 3, 4, 5, 7, 8, size - 8, size - 7, size - 6, size - 5, size - 4, size - 3, size - 2];
    const vR = [size - 1, size - 2, size - 3, size - 4, size - 5, size - 6, size - 7, 8, 7, 5, 4, 3, 2, 1, 0];
    let bestMat = null, bestScore = Infinity;
    for (let m = 0; m < 8; m++) {
      const cand = base.map((row) => row.slice());
      for (let r = 0; r < size; r++) for (let c = 0; c < size; c++) {
        if (!res[r][c]) cand[r][c] = ((cand[r][c] === 1) ^ maskFn(m, r, c)) ? 1 : -1;
      }
      const fb = formatBits(1, m);
      for (let i = 0; i < 15; i++) {
        const dark = ((fb >>> (14 - i)) & 1) === 1 ? 1 : -1;
        cand[8][hC[i]] = dark; cand[vR[i]][8] = dark;
      }
      if (ver >= 7) {
        const vb = versionBits(ver);
        for (let i = 0; i < 18; i++) {
          const d = ((vb >>> i) & 1) === 1 ? 1 : -1;
          const rr = Math.floor(i / 3), cc = size - 11 + (i % 3);
          cand[rr][cc] = d; cand[cc][rr] = d;
        }
      }
      const sc = penaltyScore(cand, size);
      if (sc < bestScore) { bestScore = sc; bestMat = cand; }
    }
    return { mat: bestMat, size, ver };
  }

  function renderQr(canvas, text) {
    const qr = buildIsoQr(text);
    const px = 300;
    canvas.width = px; canvas.height = px;
    const ctx = canvas.getContext("2d");
    ctx.fillStyle = "#ffffff"; ctx.fillRect(0, 0, px, px);
    if (!qr) return { dataUrl: canvas.toDataURL("image/png"), qr: null };
    const quiet = 4, total = qr.size + quiet * 2;
    const mod = Math.max(2, Math.floor(px / total));
    const draw = mod * total;
    const off = Math.floor((px - draw) / 2);
    ctx.fillStyle = "#000000";
    for (let r = 0; r < qr.size; r++) for (let c = 0; c < qr.size; c++) {
      if (qr.mat[r][c] === 1) ctx.fillRect(off + (c + quiet) * mod, off + (r + quiet) * mod, mod, mod);
    }
    return { dataUrl: canvas.toDataURL("image/png"), qr };
  }

  function pdfSafe(str) {
    return String(str || "").replace(/[^\x20-\x7E]/g, "_").replace(/[()\\]/g, "_");
  }

  function kreirajPdfZapis(qr, sha, qrUri, nazivPodneska) {
    const p12 = sha.slice(0, 12).toUpperCase();
    const kod = `[${sha.slice(0, 4)}-${sha.slice(4, 8)}]`.toUpperCase();
    const brojZapisa = `RUKE-${p12}`;
    const quiet = 3;
    const total = qr.size + quiet * 2;
    const boxPt = 140;
    const cell = boxPt / total;
    const x0 = 415, y0 = 640;

    let gfx = "q 1 1 1 rg 0 0 595 842 re f\n";
    gfx += "BT /F1 12 Tf 48 790 Td (ZAJEDNICA RUKE - e-SPIS QR ZAPIS PODNESKA) Tj ET\n";
    gfx += `BT /F1 10 Tf 48 770 Td (Broj zapisa: ${pdfSafe(brojZapisa)} | Kontrolni broj: ${pdfSafe(kod)}) Tj ET\n`;
    gfx += `BT /F1 9 Tf 48 752 Td (Podnesak: ${pdfSafe(nazivPodneska || `${sha.slice(0,12)}_podnesak.pdf`)}) Tj ET\n`;
    gfx += `BT /F1 8 Tf 48 736 Td (SHA-256 izvornika: ${pdfSafe(sha)}) Tj ET\n`;
    gfx += `BT /F1 8 Tf 48 720 Td (QR URI: ${pdfSafe(qrUri)}) Tj ET\n`;
    gfx += "BT /F1 8 Tf 48 704 Td (Napomena: Izvorni PDF podnesak ostaje bajtno netaknut u registru i QES-PDF bucketu.) Tj ET\n";

    gfx += `q 1 0 0 1 ${x0} ${y0} cm 1 1 1 rg 0 0 ${boxPt} ${boxPt} re f 0 0 0 rg `;
    for (let r = 0; r < qr.size; r++) {
      for (let c = 0; c < qr.size; c++) {
        if (qr.mat[r][c] === 1) {
          const rx = ((c + quiet) * cell).toFixed(2);
          const ry = ((total - 1 - (r + quiet)) * cell).toFixed(2);
          const rw = (cell + 0.05).toFixed(2);
          gfx += `${rx} ${ry} ${rw} ${rw} re f `;
        }
      }
    }
    gfx += "Q\n";

    const objs = [
      "1 0 obj << /Type /Catalog /Pages 2 0 R >> endobj\n",
      "2 0 obj << /Type /Pages /Kids [3 0 R] /Count 1 >> endobj\n",
      "3 0 obj << /Type /Page /Parent 2 0 R /MediaBox [0 0 595 842] /Resources << /Font << /F1 4 0 R >> >> /Contents 5 0 R >> endobj\n",
      "4 0 obj << /Type /Font /Subtype /Type1 /BaseFont /Helvetica >> endobj\n",
      `5 0 obj << /Length ${gfx.length} >> stream\n${gfx}endstream endobj\n`
    ];
    let out = "%PDF-1.4\n";
    const off = [0];
    for (const o of objs) { off.push(out.length); out += o; }
    const xref = out.length;
    out += `xref\n0 ${objs.length + 1}\n0000000000 65535 f \n`;
    for (let i = 1; i < off.length; i++) out += String(off[i]).padStart(10, "0") + " 00000 n \n";
    out += `trailer << /Size ${objs.length + 1} /Root 1 0 R >>\nstartxref\n${xref}\n%%EOF\n`;
    return new Blob([out], { type: "application/pdf" });
  }

  window.RukeQrZapis = {
    qrUriZaSha(sha) { return `${PORTAL_BASE}?sha256=${sha}`; },
    pripremiArtefakte(sha, nazivPodneska) {
      const qrUri = this.qrUriZaSha(sha);
      const p8 = sha.slice(0, 8).toUpperCase();
      const p12 = sha.slice(0, 12).toUpperCase();
      const kod = `[${sha.slice(0, 4)}-${sha.slice(4, 8)}]`.toUpperCase();
      const brojZapisa = `RUKE-${p12}`;
      let c = document.getElementById("qr-canvas");
      if (!c) { c = document.createElement("canvas"); c.id = "qr-canvas"; c.width = 300; c.height = 300; }
      const rendered = renderQr(c, qrUri);
      const pdfBlob = rendered.qr ? kreirajPdfZapis(rendered.qr, sha, qrUri, nazivPodneska) : null;
      return {
        sha, kod, brojZapisa, qrUri,
        ver: rendered.qr ? rendered.qr.ver : 0,
        pngDataUrl: rendered.dataUrl,
        pngIme: `QR_${p8}.png`,
        pdfBlob,
        pdfIme: `Zapis_QR_${p8}.pdf`
      };
    }
  };
})();
