#!/data/data/com.termux/files/usr/bin/bash
set -e

cd ~/njm_2_ekosustav_01

echo "[1/4] Generiranje SHA-256 sažetaka iz 01_LOKALNI_TREZOR..."
./ugradi_pin_sha256

echo "[2/4] Kompajliranje i pokretanje C23 generatora portala..."
clang -std=c23 -fsyntax-only napravi_offline_app.c
clang -std=c23 -O2 -Wall -Wextra napravi_offline_app.c -o napravi_offline_app
./napravi_offline_app

echo "[3/4] Pokretanje C23 sigurnosnog čuvara..."
./sigurnosni_cuvar

echo "[4/4] Sinkronizacija s GitHub Pages..."
git add .gitignore build_portal.sh src_c23/ napravi_offline_app.c index.html
[ -f README.md ] && git add README.md
git add -u
git commit -m "build(security): deterministički C23 build s SHA-256 PIN autentikacijom" || echo "Nema novih promjena za commit."
git push origin main

echo "[GOTOVO] Portal je sigurno ažuriran na https://zajednicaruke.github.io/e-spis/"
