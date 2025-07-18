#!/data/data/com.termux/files/usr/bin/bash

# Warna dikit biar enak dilihat (opsional)
GREEN="\033[0;32m"
RESET="\033[0m"

echo -e "${GREEN}MONERO EXPRESS 🚂 aktif... tekan CTRL+C buat berhenti${RESET}"
sleep 1

while true; do
  # Ambil angka random 0–3
  MODE=$((RANDOM % 4))

  case $MODE in
    0)
      echo "[+] Mode: Kereta Biasa"
      sl
      ;;
    1)
      echo "[+] Mode: Kereta Nabrak (Accident Mode)"
      sl -a
      ;;
    2)
      echo "[+] Mode: Kereta Terbang (Fly Mode)"
      sl -F
      ;;
    3)
      echo "[+] Mode: Slow Motion"
      sl -l
      ;;
  esac

  sleep 2
  clear
done