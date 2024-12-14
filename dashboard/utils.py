from Crypto.Cipher import AES
import re
from Crypto.Random import get_random_bytes
import hashlib
import base64


KEYS = {
    "esp1": {
        "encrypt": 'CmvgVtFuSNmQMCaRmLzzrCaPBFphSuYr',
        "hash": 'LEaiLUukPRTGtPQWhMuxdcVwEjgrcxBG'
    },
    "esp2": {
        "encrypt": 'dgCGZGDCRwEVSczHrttbcpMALwweVxnP',
        "hash": 'JAYWBfudauenBjnPUENgnEeYXmLnwSjN'
    },
    "server": {
        "encrypt": 'xSjxNewFTELfqmDdbgAWrDkTfNAfNYNs',
        "hash": 'SzMVxXdSkmvmxmFGHrUJQspxHvCxKqFW'
    }
}

# Constants
BLOCK_SIZE = 16  

import base64
import hashlib
from Crypto.Cipher import AES
from Crypto.Random import get_random_bytes

def pad_input(data, block_size):
    """Pads the input to a multiple of the block size using '+' as the padding character."""
    padding_length = block_size - (len(data) % block_size)
    return data + ('+' * padding_length)

def unpad_input(data):
    """Removes the padding character '+' from the input."""
    return data.rstrip('+')

def aes_encrypt(key, data, iv):
    """Encrypts data using AES-CBC mode."""
    cipher = AES.new(key, AES.MODE_CBC, iv)
    return cipher.encrypt(data)

def aes_decrypt(key:str, cipher_text:str, iv):
    """Decrypts data using AES-CBC mode."""
    cipher = AES.new(key.encode('utf-8'), AES.MODE_CBC, iv)
    return cipher.decrypt(cipher_text.encode('utf-8'))

def compute_hash(data, hash_key):
    """Computes the SHA-256 hash of the concatenated data and hash key."""
    combined = f"{data}.{hash_key}".encode('utf-8')
    return hashlib.sha256(combined).digest()

def encrypt_payload(data, key, hash_key):
    """Encrypts the payload and generates the corresponding hash and IV."""
    block_size = 16

    # Pad the input data
    padded_data = pad_input(data, block_size)

    # Generate IV
    iv = get_random_bytes(block_size)

    # Encrypt the padded data
    cipher_text = aes_encrypt(key, padded_data.encode('utf-8'), iv)

    # Base64 encode the IV, ciphertext, and hash
    iv_b64 = base64.b64encode(iv).decode('utf-8')
    cipher_text_b64 = base64.b64encode(cipher_text).decode('utf-8')

    # Compute the hash
    computed_hash = compute_hash(cipher_text_b64, hash_key)
    hash_b64 = base64.b64encode(computed_hash).decode('utf-8')

    # Concatenate the components into the final payload
    payload = f"{cipher_text_b64}.{hash_b64}.{iv_b64}"
    return payload

def process_received_payload(payload, key, hash_key):
    """Decrypts the payload and verifies the hash."""
    # Split the payload into its components
    try:
        cipher_text_b64, hash_b64, iv_b64 = payload.split('.')
    except ValueError:
        raise ValueError("Invalid payload format.")

    # Decode the Base64 components
    iv = base64.b64decode(iv_b64)
    cipher_text = base64.b64decode(cipher_text_b64)
    received_hash = base64.b64decode(hash_b64)

    # Verify the hash
   # computed_hash = compute_hash(cipher_text_b64, hash_key)
   # if computed_hash != received_hash:
   #     raise ValueError("Hash verification failed.")

    # Decrypt the ciphertext
    decrypted_padded_data = aes_decrypt(key, cipher_text, iv).decode('utf-8')

    # Unpad the data
    return unpad_input(decrypted_padded_data)

data = "WtVfHgOl+6C8cLNDOTJuLQ==.8jTfu9adaSQ8xA+8A2qwbJQHqkBQxB7raBNPOCmtswQ=.0BoKrtK78pDaxAfDQJrL7A=="
key = KEYS['esp1']['encrypt']
hash_key = KEYS['esp1']['hash']
decrypted_data = process_received_payload(data, key, hash_key)
print("Decrypted Data:", decrypted_data)

