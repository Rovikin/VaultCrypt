#include <sodium.h>
#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <sstream>
#include <iomanip>

// Charset ASCII aman
const std::string SAFE_ASCII = [] {
    std::string s;
    for (int i = 33; i < 127; ++i)
        if (i != '"' && i != '\'' && i != '`' && i != '\\')
            s += static_cast<char>(i);
    return s;
}();

int count_bits(char c) {
    int count = 0;
    for (int i = 0; i < 8; ++i)
        if (c & (1 << i)) ++count;
    return count;
}

std::string sha512(const std::string& input) {
    unsigned char hash[crypto_hash_sha512_BYTES];
    crypto_hash_sha512(hash, (const unsigned char*)input.data(), input.size());
    return std::string((char*)hash, crypto_hash_sha512_BYTES);
}

std::string hmac_sha512(const std::string& key, const std::string& data) {
    unsigned char out[crypto_auth_hmacsha512_BYTES];
    crypto_auth_hmacsha512_state state;
    crypto_auth_hmacsha512_init(&state, (const unsigned char*)key.data(), key.size());
    crypto_auth_hmacsha512_update(&state, (const unsigned char*)data.data(), data.size());
    crypto_auth_hmacsha512_final(&state, out);
    return std::string((char*)out, crypto_auth_hmacsha512_BYTES);
}

std::string character_bit_chain(std::string state, const std::string& password) {
    for (size_t i = 0; i < password.size(); ++i) {
        char ch = password[i];
        int bits = count_bits(ch) + (i % 7);
        state += ch;
        for (int j = 0; j < bits; ++j)
            state = sha512(state + std::to_string(j));
    }
    return state;
}

std::string derive_key_argon2(const std::string& password, const std::string& salt_str) {
    unsigned char key[64];
    unsigned char salt[crypto_pwhash_SALTBYTES] = {0};

    // Copy salt string into fixed-length salt
    size_t copy_len = std::min(salt_str.size(), sizeof(salt));
    memcpy(salt, salt_str.data(), copy_len);

    if (crypto_pwhash(key, sizeof(key),
                      password.c_str(), password.size(),
                      salt,
                      4,        // opslimit
                      64 * 1024 * 1024,  // memlimit: 64MB
                      crypto_pwhash_ALG_ARGON2ID13) != 0) {
        std::cerr << "❌ Derivasi Argon2 gagal. RAM tidak cukup?\n";
        exit(1);
    }

    return std::string((char*)key, sizeof(key));
}

std::string encode_baseX(const std::string& data, const std::string& charset = SAFE_ASCII) {
    std::string encoded;
    size_t base = charset.size();
    std::vector<unsigned char> bytes(data.begin(), data.end());
    unsigned long long num = 0;
    for (unsigned char b : bytes)
        num = (num << 8) + b;

    while (num > 0) {
        encoded = charset[num % base] + encoded;
        num /= base;
    }
    return encoded;
}

std::string ensure_min_length(std::string encoded, size_t min_length = 4096) {
    std::string original = encoded;
    while (encoded.size() < min_length) {
        std::string digest = hmac_sha512(encoded, original);
        encoded += encode_baseX(digest);
    }
    return encoded.substr(0, min_length);
}

int main() {
    if (sodium_init() < 0) {
        std::cerr << "❌ libsodium gagal diinisialisasi.\n";
        return 1;
    }

    std::cout << "🔐 ULTRA PASSWORD GENERATOR — Mode Argon2 + HMAC + SHA512\n\n";

    std::string password, salt;
    std::cout << "Masukkan password: ";
    std::getline(std::cin, password);
    std::cout << "Masukkan salt: ";
    std::getline(std::cin, salt);

    std::cout << "\n[*] Derivasi password... mohon tunggu...\n";

    // Tahap 1: Argon2
    std::string argon_key = derive_key_argon2(password, salt);

    // Tahap 2: Bit chaining
    std::string chained = character_bit_chain(argon_key, password);

    // Tahap 3: HMAC + SHA512 mixing
    std::string mixed = hmac_sha512(chained, salt);

    // Tahap 4: Encoding baseX
    std::string encoded = encode_baseX(mixed);

    // Tahap 5: Extend panjangnya
    std::string final_output = ensure_min_length(encoded, 4096);

    std::cout << "\n✅ Password Ultra-Kuat Siap!\n";
    std::cout << "📏 Panjang: " << final_output.size() << " karakter\n\n";
    std::cout << final_output << "\n";

    std::this_thread::sleep_for(std::chrono::seconds(15));
    std::cout << "\033[2J\033[1;1H";  // clear screen
    return 0;
}
