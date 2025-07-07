🔐 VaultCrypt - Enkripsi File Kuat, Ringan, & Aman
=================================================

**VaultCrypt** adalah script Python ringan yang mengamankan file pribadi (seperti seed phrase, dokumen rahasia, dll) menggunakan enkripsi **AES-256-GCM** yang dipadukan dengan **Argon2id** — salah satu key derivation function paling kuat saat ini.

Dirancang khusus untuk berjalan mulus di **Termux (Android)** tanpa root, simpel, dan *no trace left behind*.

---

✨ Fitur Utama
-------------

* 🔒 AES-256 GCM (authenticated encryption)
* 🧠 Argon2id KDF dengan 256MB RAM & 4 threads
* ✅ Checksum SHA256 untuk verifikasi integritas
* 🧂 Salt unik per file
* 💣 File plaintext langsung ditimpa
* 📁 Tidak butuh keyfile eksternal

📦 Instalasi di Termux
---------------------

```
pkg update && pkg upgrade
pkg install python clang openssl libffi
pip install pycryptodome argon2-cffi

```

🚀 Cara Penggunaan
-----------------

### 🔐 Enkripsi

```
python enc2.py
(E)ncrypt or (D)ecrypt ? e
Input your file: rahasia.txt
Input your passphrase: ****
Confirm your passphrase: ****
```

### 🔓 Dekripsi

```
python enc2.py
(E)ncrypt or (D)ecrypt ? d
Input your file: rahasia.txt
Input your passphrase: ****
```

⚠️ **Catatan:** File plaintext akan langsung ditimpa setelah dienkripsi. Tidak ada backup otomatis.

🛡️ Keamanan
-----------

* ✅ AES-256 + GCM = aman + integritas
* 🧠 Argon2id = KDF anti brute-force
* 🔐 SHA256 = verifikasi & anti tamper
* 🔥 Tidak ada file kunci eksternal → hanya passphrase

💡 Rekomendasi
-------------

* Gunakan passphrase kuat (12+ karakter random)
* Uji coba dulu sebelum pakai untuk data penting
* Backup offline tetap direkomendasikan

📜 Lisensi
---------

MIT License — bebas digunakan & dimodifikasi. Gunakan dengan tanggung jawab.