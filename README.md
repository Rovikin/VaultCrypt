# 🔐 VaultCrypt – Enkripsi Seed Phrase dengan Argon2 + XChaCha20
secure_seed adalah tool enkripsi dan dekripsi seed phrase (frasa pemulihan wallet) atau file apapun dengan keamanan tinggi. Dirancang untuk berjalan di Termux (Android), dan menggunakan algoritma modern: Argon2id + XChaCha20-Poly1305.

## 🚀 Cara Install di Termux
### untuk kode C++
```pkg update && pkg upgrade -y
pkg install git clang libsodium make
git clone https://github.com/Rovikin/VaultCrypt.git
cd VaultCrypt
```

# 📂 Struktur File

##### main.cpp
##### makefile 
##### README.md

# 🛠️ Build Tool

Cukup jalankan:

```
make
```

Ini akan menghasilkan file: 

```
secure_seed
```

# 🔐 Cara Enkripsi

buat file dengan perintah misalnya:
```
nano seed
```
Lalu isi teks seed phrase kedalamnya, misalnya:

```
treat health ankle sudden brick state mercy hamster attend twenty require risk
```
Tekan ctrl + x + y

Jalankan program:
```
./secure_seed

```
Pilih opsi
``` 
E (enkripsi)
```
Masukkan nama file misalnya:
```
 seed
```

Masukkan dan konfirmasi passphrase
Output terenkripsi akan disimpan sebagai
```
seed.enc
```

# 🔓 Cara Dekripsi
Jalankan:
```
./secure_seed
```
Pilih opsi:
```
D (dekripsi)
```
Masukkan nama file misalnya:
```
seed.enc
```
Masukkan passphrase
Jika benar, seed phrase akan ditampilkan di layar

#### Contoh hasil encrypt & decrypt:
```
~/enc $ ./vault
🔐 (E)ncrypt or (D)ecrypt? [E/D]: e
📂 Enter file to encrypt: seed
[*] Reading file...
🔑 Enter passphrase:
🔁 Confirm passphrase:
[*] Generating salt and nonce...
[*] Deriving key with Argon2id (this may take a while)            [*] Locking key in memory...
[*] Encrypting...
[*] Packing encrypted data...
[*] Writing to file...
------------------------------------------------------------
✅ Encrypted → seed.enc

~/enc $ ./secure_seed
🔐 (E)ncrypt or (D)ecrypt? [E/D]: d
📂 Enter encrypted file (.enc): seed.enc
[*] Reading encrypted file...
[*] Checking magic bytes...
[*] Extracting salt and nonce...
🔑 Enter passphrase:
[*] Deriving key with Argon2id (this may take a while)
[*] Locking key...
[*] Decrypting...
------------------------------------------------------------
===========================================
treat health ankle sudden brick state mercy hamster attend twenty require risk

===========================================
~/enc $
```
### ⚠️ Catatan: Hanya passphrase yang diperlukan untuk membuka file. Tidak ada keyfile. Jangan pernah lupa passphrase atau kehilangan file terenkripsi.
### 🔥 Penting: File terenkripsi akan terhapus otomatis jika salah memasukkan passphrase 3 kali.
# 🔒 Teknologi yang Digunakan
Argon2id – fungsi derivasi kunci yang tahan terhadap brute-force dan side-channel
XChaCha20-Poly1305 – cipher stream authenticated encryption modern
libsodium – pustaka kriptografi modern & battle-tested

# 📣 Saran
Gunakan passphrase acak minimal 12 karakter ( gabungan huruf besar/kecil + angka + simbol)
Simpan hasil terenkripsi di folder aman, backup ke dua tempat
Jangan pernah menyimpan passphrase di tempat yang tidak terenkripsi! Atau lebih aman untuk menggunakan passphrase yang anda hafalkan diluar kepala namun tetap dengan gabungan huruf besar + kecil + angka + karakter.
# 🛡️ Dibuat untuk melindungi hal paling berharga di dunia kripto: frasa pemulihan.

# Untuk python
implementasinya jauh lebih sederhana. Ikuti baris perintah berikut ini:
```
pkg update && pkg upgrade -y
pkg install python git
pip install pycryptodome
pip install argon2-cffi
git clone https://github.com/Rovikin/VaultCrypt.git
cd VaultCrypt
```

jalankan program 
```
python enc.py
```

lalu ikuti instruksi seperti yang ada pada program diatas.

## Password Hash Generator

Tool ini adalah untuk menderivasi plaintext password dengan hashing plaintext dengan argon2id untuk derivasi, SHA-512 + HMAC sebagai chaining tambahan, encoding ke BaseX (ASCII printable), menjadikan outputnya menjadi 4096 karakter.
Tidak perlu menyimpan hasil derivasi password. Cukup ingat password dan salt. Simpan file 'password_hash_generator.cpp' atau kunjungi repo ini untuk mendapatkannya lagi.
##### Instruksi
Untuk kompilasi cukup ikuti perintah ini:
```
g++ -o pwhash_gen password_hash_generator.cpp -lsodium
./pwhash_gen
```

### Saran dan kritik
Saran dan kritik anda akan sangat bermanfaat bagi saya. Terimakasih
