# 🔐 VaultCrypt – Enkripsi Seed Phrase dengan Argon2 + XChaCha20
secure_seed adalah tool enkripsi dan dekripsi seed phrase (frasa pemulihan wallet) atau file apapun dengan keamanan tinggi. Dirancang untuk berjalan di Termux (Android), dan menggunakan algoritma modern: Argon2id + XChaCha20-Poly1305.

## 🚀 Cara Install di Termux
```pkg update && pkg upgrade -y
pkg install git clang libsodium make
git clone https://github.com/Rovikin/VaultCrypt.git
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
canvas amount poverty major solid stereo shallow provide wonder enhance base neglect
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
Jika salah 3x, file akan dihapus otomatis!
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

Stay paranoid. Stay sovereign. 🧠⚔️