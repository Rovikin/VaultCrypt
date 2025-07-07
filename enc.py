import os
import getpass
import hashlib
from Crypto.Cipher import AES
from Crypto.Random import get_random_bytes
from argon2.low_level import hash_secret_raw, Type

# Konstanta
SALT_SIZE = 16
NONCE_SIZE = 12
KEY_SIZE = 32
TAG_SIZE = 16
HASH_SIZE = 32  # SHA256 output

# Argon2 konfigurasi diperkuat
ARGON2_TIME_COST = 6
ARGON2_MEMORY_COST = 262144  # 256 MB
ARGON2_PARALLELISM = 4

def derive_key(passphrase: str, salt: bytes) -> bytes:
    raw_key = hash_secret_raw(
        secret=passphrase.encode(),
        salt=salt,
        time_cost=ARGON2_TIME_COST,
        memory_cost=ARGON2_MEMORY_COST,
        parallelism=ARGON2_PARALLELISM,
        hash_len=KEY_SIZE,
        type=Type.ID
    )
    return hashlib.sha256(raw_key).digest()

def encrypt_file(filename):
    passphrase = getpass.getpass("Input your passphrase: ")
    confirm = getpass.getpass("Confirm your passphrase: ")

    if passphrase != confirm:
        print("❌ Passphrase doesn't match. Abort.")
        return

    if not os.path.exists(filename):
        print("❌ File not found.")
        return

    with open(filename, "rb") as f:
        plaintext = f.read()

    salt = get_random_bytes(SALT_SIZE)
    nonce = get_random_bytes(NONCE_SIZE)
    key = derive_key(passphrase, salt)

    # Tambahkan SHA256 checksum ke plaintext
    checksum = hashlib.sha256(plaintext).digest()
    final_plaintext = checksum + plaintext

    cipher = AES.new(key, AES.MODE_GCM, nonce=nonce)
    ciphertext, tag = cipher.encrypt_and_digest(final_plaintext)

    # Format: [salt][nonce][tag][ciphertext]
    encrypted_data = salt + nonce + tag + ciphertext

    with open(filename, "wb") as f:
        f.write(encrypted_data)

    print("✅ File encrypted and overwritten securely.")

def decrypt_file(filename):
    passphrase = getpass.getpass("Input your passphrase: ")

    if not os.path.exists(filename):
        print("❌ File not found.")
        return

    with open(filename, "rb") as f:
        file_data = f.read()

    try:
        salt = file_data[:SALT_SIZE]
        nonce = file_data[SALT_SIZE:SALT_SIZE + NONCE_SIZE]
        tag = file_data[SALT_SIZE + NONCE_SIZE:SALT_SIZE + NONCE_SIZE + TAG_SIZE]
        ciphertext = file_data[SALT_SIZE + NONCE_SIZE + TAG_SIZE:]

        key = derive_key(passphrase, salt)

        cipher = AES.new(key, AES.MODE_GCM, nonce=nonce)
        decrypted = cipher.decrypt_and_verify(ciphertext, tag)

        # Pisahkan checksum & isi asli
        checksum = decrypted[:HASH_SIZE]
        plaintext = decrypted[HASH_SIZE:]

        # Verifikasi checksum
        if hashlib.sha256(plaintext).digest() != checksum:
            print("❌ Checksum mismatch! File corrupted or tampered.")
            return

        print("✅ Decryption successful.\n")
        print("=" * 43)
        print(plaintext.decode(errors="replace"))
        print("=" * 43 + "\n")

    except Exception as e:
        print("❌ Decryption failed:", str(e))

def main():
    print("(E)ncrypt or (D)ecrypt ?")
    choice = input("Your choice [E/D]: ").strip().lower()

    if choice not in ('e', 'd'):
        print("❌ Invalid choice.")
        return

    filename = input("Input your file: ").strip()

    if choice == 'e':
        encrypt_file(filename)
    else:
        decrypt_file(filename)

if __name__ == "__main__":
    main()
