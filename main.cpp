#include <sodium.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <unistd.h>
#include <sys/mman.h>
#include <cstring>

#define MAGIC "\xA1\xB2\xC3\xD4"
#define MAGIC_LEN 4
#define SALT_LEN crypto_pwhash_SALTBYTES
#define NONCE_LEN crypto_aead_xchacha20poly1305_ietf_NPUBBYTES
#define KEY_LEN crypto_aead_xchacha20poly1305_ietf_KEYBYTES
#define TAG_LEN crypto_aead_xchacha20poly1305_ietf_ABYTES
#define MAX_RETRY 3

#define ARGON2_RAM_LIMIT (1024ULL * 1024 * 1024) // 1 GB RAM
#define ARGON2_TIME_COST 6
#define ARGON2_PARALLELISM 4

std::string prompt_hidden(const std::string& label) {
    std::cout << label;
    system("stty -echo");
    std::string line; std::getline(std::cin, line);
    system("stty echo"); std::cout << "\n";
    return line;
}

std::vector<unsigned char> read_file(const std::string& f) {
    std::ifstream ifs(f, std::ios::binary);
    return std::vector<unsigned char>((std::istreambuf_iterator<char>(ifs)), {});
}

void write_file(const std::string& f, const std::vector<unsigned char>& v) {
    std::ofstream ofs(f, std::ios::binary);
    ofs.write((char*)v.data(), v.size());
}

bool derive_key(unsigned char *key, const std::string& pass, const unsigned char *salt) {
    return crypto_pwhash(key, KEY_LEN,
        pass.c_str(), pass.size(),
        salt,
        ARGON2_TIME_COST,
        ARGON2_RAM_LIMIT,
        crypto_pwhash_ALG_ARGON2ID13) == 0;
}

void secure_zero(void* p, size_t n) {
    if (p && n) sodium_memzero(p, n);
}

void encrypt_seed() {
    std::string infile;
    std::cout << "📂 Masukkan nama file seed phrase: ";
    std::getline(std::cin, infile);

    auto msg = read_file(infile);
    if (msg.empty()) {
        std::cerr << "❌ File kosong atau tidak ditemukan.\n";
        return;
    }

    auto pass = prompt_hidden("🔑 Passphrase: ");
    auto confirm = prompt_hidden("🔁 Konfirmasi passphrase: ");
    if (pass != confirm) {
        std::cerr << "❌ Passphrase tidak cocok!\n"; return;
    }
    if (pass.size() < 12) {
        std::cerr << "❌ Panjang passphrase minimal 12 karakter.\n"; return;
    }

    unsigned char salt[SALT_LEN];
    randombytes_buf(salt, SALT_LEN);
    unsigned char nonce[NONCE_LEN];
    randombytes_buf(nonce, NONCE_LEN);

    unsigned char key[KEY_LEN];
    if (!derive_key(key, pass, salt)) {
        std::cerr << "❌ Gagal derive key.\n"; return;
    }

    mlock(key, KEY_LEN);

    std::vector<unsigned char> ct(msg.size() + TAG_LEN);
    unsigned long long clen;

    if (crypto_aead_xchacha20poly1305_ietf_encrypt(ct.data(), &clen,
        msg.data(), msg.size(), nullptr, 0, nullptr, nonce, key) != 0) {
        std::cerr << "❌ Encrypt error\n";
        secure_zero(key, KEY_LEN); munlock(key, KEY_LEN); return;
    }

    std::string outname = infile + ".enc";
    std::vector<unsigned char> out;
    out.insert(out.end(), (unsigned char*)MAGIC, (unsigned char*)MAGIC + MAGIC_LEN);
    out.insert(out.end(), salt, salt + SALT_LEN);
    out.insert(out.end(), nonce, nonce + NONCE_LEN);
    out.insert(out.end(), ct.begin(), ct.begin() + clen);

    write_file(outname, out);
    std::cout << "✅ Terenkripsi ke: " << outname << "\n";

    secure_zero(key, KEY_LEN); munlock(key, KEY_LEN);
}

void decrypt_seed() {
    std::string infile;
    std::cout << "📂 Masukkan nama file terenkripsi (.enc): ";
    std::getline(std::cin, infile);
    auto dat = read_file(infile);
    if (dat.size() < MAGIC_LEN + SALT_LEN + NONCE_LEN + TAG_LEN ||
        memcmp(dat.data(), MAGIC, MAGIC_LEN) != 0) {
        std::cerr << "❌ File rusak atau bukan format valid.\n"; return;
    }

    unsigned char salt[SALT_LEN];
    memcpy(salt, dat.data() + MAGIC_LEN, SALT_LEN);
    unsigned char nonce[NONCE_LEN];
    memcpy(nonce, dat.data() + MAGIC_LEN + SALT_LEN, NONCE_LEN);
    std::vector<unsigned char> ct(dat.begin() + MAGIC_LEN + SALT_LEN + NONCE_LEN, dat.end());

    int tries = 0;
    while (tries < MAX_RETRY) {
        auto pass = prompt_hidden("🔑 Masukkan passphrase: ");
        unsigned char key[KEY_LEN];
        if (!derive_key(key, pass, salt)) {
            std::cerr << "❌ Gagal derive key.\n"; return;
        }

        mlock(key, KEY_LEN);
        std::vector<unsigned char> pt(ct.size());
        unsigned long long plen;
        int ok = crypto_aead_xchacha20poly1305_ietf_decrypt(
            pt.data(), &plen, nullptr, ct.data(), ct.size(), nullptr, 0, nonce, key);

        secure_zero(key, KEY_LEN); munlock(key, KEY_LEN);

        if (ok == 0) {
            std::cout << "✅ SEED:\n================================\n"
                      << std::string(pt.begin(), pt.begin() + plen)
                      << "\n================================\n";
            return;
        } else {
            std::cerr << "❌ Passphrase salah!\n";
            tries++;
        }
    }

    std::cerr << "⚠️ Salah 3x. File dienkripsi dihapus!\n";
    unlink(infile.c_str());
}

int main() {
    if (sodium_init() < 0) {
        std::cerr << "❌ libsodium gagal diinisialisasi\n";
        return 1;
    }
    std::string ch;
    std::cout << "🔐 (E)nkripsi atau (D)ekripsi? [E/D]: ";
    std::getline(std::cin, ch);
    if (ch == "E" || ch == "e") encrypt_seed();
    else if (ch == "D" || ch == "d") decrypt_seed();
    else std::cerr << "❌ Pilihan tidak valid\n";
    return 0;
}
