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
IV_SIZE = 12
TAG_SIZE = 16
BLOCK_SIZE = 16  # For PKCS#7 padding

# Nonce counters for replay protection for ESP devices
nonce_counters = {
    'esp1': 0,
    'esp2': 0,
    'server': 0  # Separate counter for server nonce, generated independently
}

def pad_input(input):
    input_length = len(input)
    padding_length = BLOCK_SIZE - (input_length % BLOCK_SIZE)
    padded_input = input + chr(padding_length) * padding_length
    return padded_input

def unpad_output(decrypted: bytes) -> str:
    """
    Extracts numeric parts (digits and optional periods) from decrypted data.

    :param decrypted_data: The decrypted data as bytes.
    :return: The extracted numeric part as a string.
    """
    # Decode the decrypted data to string safely (ignoring non-UTF-8 bytes)
    decrypted_str = decrypted.decode("utf-8", errors="ignore")
    # Use a regex to find numbers, including decimals
    match = re.search(r"\d+(\.\d+)?", decrypted_str)
    
    return match.group(0) if match else ""

def aes_decrypt(key:str, iv, ciphertext):
    cipher = AES.new(base64.b64decode(key), AES.MODE_CBC, iv)
    decrypted = cipher.decrypt(ciphertext)

    print(decrypted)
    original_data = unpad_output(decrypted)
    return original_data

def aes_encrypt(key:str, iv:bytes, message:bytes):
    cipher = AES.new(base64.b64decode(key), AES.MODE_CBC, iv=iv)
    message_to_cipher = pad_input(message)
    return cipher.encrypt(bytes(message_to_cipher, "utf-8")).digest()
    
def process_received_payload(payload, key, hmac_key):
    parts = payload.split('.')
    if len(parts) != 3:
        raise ValueError("Invalid payload format")
    
    ciphertext_b64, hmac_b64, iv_b64 = parts
    ciphertext = base64.b64decode(ciphertext_b64)

    if len(ciphertext) % BLOCK_SIZE != 0:
        raise ValueError("Unable to decrypt data")
    
    hmac = base64.b64decode(hmac_b64)
    iv = base64.b64decode(iv_b64)

    # Decrypt the ciphertext
    decrypted_data = aes_decrypt(key, iv, ciphertext)
    print(decrypted_data)

    return float(decrypted_data)

def prepare_payload(data, key, hash_key):
    padded_data = pad_input(data)
    iv = get_random_bytes(IV_SIZE)
    ciphertext = aes_encrypt(key, iv, padded_data)
    hash = compute_hmac(ciphertext, hash_key)
    iv_b64 = base64.b64encode(iv)
    hash_b64 = base64.b64encode(hash)
    ciphertext_b64 = base64.b64encode(ciphertext)
    return f"{ciphertext_b64}.{hash_b64}.{iv_b64}"

data = "ZWj5Pn1M32SDE1dMTWJqgg==.SQlPfj4dR0QTU7voI7IbYz5E8mRrWP33dMIFAQ+p7HU=.G1f7/ud/cj8bD7iluKBQWA=="
key = KEYS['esp1']['encrypt']
hmac_key = KEYS['esp1']['hash']
decrypted_data = process_received_payload(data, key, hmac_key)
print("Decrypted Data:", decrypted_data)

