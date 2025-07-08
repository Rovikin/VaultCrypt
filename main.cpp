#include <sodium.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <termios.h>
#include <unistd.h>
#include <cstring>
#include <sys/mman.h>
#include <chrono>
#include <thread>

#define MAGIC "\xDE\xAD\xBE\xEF"
#define MAGIC_LEN 4
#define SALT_LEN crypto_pwhash_SALTBYTES
#define NONCE_LEN crypto_aead_xchacha20poly1305_ietf_NPUBBYTES
#define KEY_LEN crypto_aead_xchacha20poly1305_ietf_KEYBYTES
#define TAG_LEN crypto_aead_xchacha20poly1305_ietf_ABYTES

#define ARGON2_RAM_LIMIT (1024ULL * 1024 * 1024) // 1GB RAM
#define ARGON2_TIME_COST 6
#define ARGON2_PARALLELISM 4

void print_centered(const std::string& msg) {
    std::cout << std::string(43, '=') << "\n";
    std::cout << msg << "\n";
    std::cout << std::string(43, '=') << "\n";
}

void print_log(const std::string& msg) {
    std::cout << "[*] " << msg << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
}

void show_loading(const std::string& message, int duration_ms = 4000) {
    const char spinner[] = "|/-\\";
    std::cout << "[*] " << message << " ";
    std::cout.flush();

    auto start = std::chrono::steady_clock::now();
    int i = 0;
    while (std::chrono::steady_clock::now() - start < std::chrono::milliseconds(duration_ms)) {
        std::cout << "\b" << spinner[i++ % 4];
        std::cout.flush();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    std::cout << "\b \n";
}

void clear_screen_preserve_logs() {
    std::cout << "\n\n" << std::string(60, '-') << "\n";
}

std::string prompt_hidden(const std::string& prompt) {
    std::cout << prompt;
    termios oldt, newt;
    std::string password;

    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    std::getline(std::cin, password);
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    std::cout << std::endl;
    return password;
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

bool derive_key_verbose(unsigned char *key, const std::string& pass, const unsigned char *salt) {
    show_loading("Deriving key with Argon2id (this may take a while)", 4000);
    return derive_key(key, pass, salt);
}

void secure_zero(void* p, size_t n) {
    if (p && n) sodium_memzero(p, n);
}

bool lock_key(unsigned char *key, size_t len) {
    return mlock(key, len) == 0;
}

void unlock_key(unsigned char *key, size_t len) {
    munlock(key, len);
}

void encrypt_file() {
    std::string infile;
    std::cout << "📂 Enter file to encrypt: ";
    std::getline(std::cin, infile);

    print_log("Reading file...");
    auto msg = read_file(infile);
    if (msg.empty()) {
        std::cerr << "❌ File not found or empty.\n";
        return;
    }

    auto pass1 = prompt_hidden("🔑 Enter passphrase: ");
    auto pass2 = prompt_hidden("🔁 Confirm passphrase: ");
    if (pass1 != pass2) {
        std::cerr << "❌ Passphrases do not match.\n";
        return;
    }
    if (pass1.size() < 12) {
        std::cerr << "❌ Passphrase must be at least 12 characters.\n";
        return;
    }

    print_log("Generating salt and nonce...");
    unsigned char salt[SALT_LEN];
    randombytes_buf(salt, SALT_LEN);
    unsigned char nonce[NONCE_LEN];
    randombytes_buf(nonce, NONCE_LEN);
    unsigned char key[KEY_LEN];

    if (!derive_key_verbose(key, pass1, salt)) {
        std::cerr << "❌ Key derivation failed.\n";
        return;
    }

    print_log("Locking key in memory...");
    if (!lock_key(key, KEY_LEN)) {
        std::cerr << "❌ Failed to lock memory.\n";
        secure_zero(key, KEY_LEN);
        return;
    }

    print_log("Encrypting...");
    std::vector<unsigned char> ct(msg.size() + TAG_LEN);
    unsigned long long clen;

    if (crypto_aead_xchacha20poly1305_ietf_encrypt(
        ct.data(), &clen,
        msg.data(), msg.size(),
        nullptr, 0, nullptr, nonce, key) != 0) {
        std::cerr << "❌ Encryption failed.\n";
        secure_zero(key, KEY_LEN); unlock_key(key, KEY_LEN); return;
    }

    print_log("Packing encrypted data...");
    std::vector<unsigned char> out;
    out.insert(out.end(), (unsigned char*)MAGIC, (unsigned char*)MAGIC + MAGIC_LEN);
    out.insert(out.end(), salt, salt + SALT_LEN);
    out.insert(out.end(), nonce, nonce + NONCE_LEN);
    out.insert(out.end(), ct.begin(), ct.begin() + clen);

    print_log("Writing to file...");
    write_file(infile + ".enc", out);

    clear_screen_preserve_logs();
    std::cout << "✅ Encrypted → " << infile << ".enc\n";

    secure_zero(key, KEY_LEN);
    unlock_key(key, KEY_LEN);
}

bool decrypt_file() {
    std::string infile;
    std::cout << "📂 Enter encrypted file (.enc): ";
    std::getline(std::cin, infile);

    print_log("Reading encrypted file...");
    auto dat = read_file(infile);
    if (dat.size() < MAGIC_LEN + SALT_LEN + NONCE_LEN + TAG_LEN) {
        std::cerr << "❌ Invalid or too short file.\n";
        return false;
    }

    print_log("Checking magic bytes...");
    if (memcmp(dat.data(), MAGIC, MAGIC_LEN) != 0) {
        std::cerr << "❌ Invalid file magic.\n";
        return false;
    }

    print_log("Extracting salt and nonce...");
    unsigned char salt[SALT_LEN];
    memcpy(salt, dat.data() + MAGIC_LEN, SALT_LEN);
    unsigned char nonce[NONCE_LEN];
    memcpy(nonce, dat.data() + MAGIC_LEN + SALT_LEN, NONCE_LEN);
    std::vector<unsigned char> ct(dat.begin() + MAGIC_LEN + SALT_LEN + NONCE_LEN, dat.end());

    for (int attempts = 0; attempts < 5; ++attempts) {
        auto pass = prompt_hidden("🔑 Enter passphrase: ");

        unsigned char key[KEY_LEN];
        if (!derive_key_verbose(key, pass, salt)) {
            std::cerr << "❌ Key derivation failed.\n";
            return false;
        }

        print_log("Locking key...");
        if (!lock_key(key, KEY_LEN)) {
            std::cerr << "❌ Failed to lock memory.\n";
            secure_zero(key, KEY_LEN);
            return false;
        }

        print_log("Decrypting...");
        std::vector<unsigned char> pt(ct.size());
        unsigned long long plen;
        int ok = crypto_aead_xchacha20poly1305_ietf_decrypt(
            pt.data(), &plen, nullptr, ct.data(), ct.size(), nullptr, 0, nonce, key);

        secure_zero(key, KEY_LEN); unlock_key(key, KEY_LEN);

        if (ok == 0) {
            clear_screen_preserve_logs();
            std::string result(pt.begin(), pt.begin() + plen);
            print_centered(result);
            return true;
        } else {
            std::cerr << "❌ Wrong passphrase. Attempts left: " << (4 - attempts) << "\n";
        }
    }

    std::cerr << "🚫 Too many failed attempts. Aborted.\n";
    return false;
}

int main() {
    if (sodium_init() < 0) {
        std::cerr << "❌ libsodium init failed.\n";
        return 1;
    }

    std::string choice;
    std::cout << "🔐 (E)ncrypt or (D)ecrypt? [E/D]: ";
    std::getline(std::cin, choice);

    if (choice == "E" || choice == "e" || choice == "1") encrypt_file();
    else if (choice == "D" || choice == "d" || choice == "2") decrypt_file();
    else std::cerr << "❌ Invalid choice.\n";

    return 0;
}
