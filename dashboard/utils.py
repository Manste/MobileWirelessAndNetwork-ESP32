from Crypto.Cipher import AES
import re
from Crypto.Random import get_random_bytes
import hashlib
import base64


KEYS = {
    "esp1": {
        "encrypt": 'H50Eoq18OKL+QcpnUNS4p7qwYLWBzWUZm0V+n85mjLI=',
        "hash": 'LEaiLUukPRTGtPQWhMuxdcVwEjgrcxBG'
    },
    "esp2": {
        "encrypt": 't6PeycMhrW003pbO2W8YPI54jt18BZmqIfwpxHgZbyQ=',
        "hash": 'JAYWBfudauenBjnPUENgnEeYXmLnwSjN'
    },
    "server": {
        "encrypt": 'fTmJ6IfIHA+ebpztbkePGgZ7J2Dxd1xxDF9aHSjb0FI=',
        "hash": 'SzMVxXdSkmvmxmFGHrUJQspxHvCxKqFW'
    }
}

# Constants
BLOCK_SIZE = 16

def pad_input(input:str) -> bytes:
    input_length = len(input)
    padding_length = BLOCK_SIZE - (input_length % BLOCK_SIZE)
    padding = bytes([padding_length] * padding_length) 
    padded_input = input.encode("utf-8") + padding
    return padded_input

def unpad_output(decrypted: bytes) -> str:
    padding_length = decrypted[-1]
    original_length = len(decrypted) - padding_length
    return decrypted[:original_length].decode('utf-8')

def aes_decrypt(key:str, iv, ciphertext:bytes)->str:
    cipher = AES.new(base64.b64decode(key), AES.MODE_CBC, iv)
    decrypted = cipher.decrypt(ciphertext)
    original_data = unpad_output(decrypted)
    return original_data

def aes_encrypt(key:str, iv:bytes, message:str):
    cipher = AES.new(base64.b64decode(key), AES.MODE_CBC, iv=iv)
    message_to_cipher = pad_input(message)
    return cipher.encrypt(message_to_cipher)

def compute_hash(data:str, key:str) -> str:
    h = hashlib.sha256(bytes(data + "." + key, "utf-8"))
    return h.digest()

def verify_hash(data:str, received_hash:bytes, key:str) -> bool:
    computed_hash = compute_hash(data, key)
    return computed_hash == received_hash

def process_received_payload(payload, esp_name):
    key = KEYS[esp_name]["encrypt"]
    hash_key = KEYS[esp_name]["hash"]

    parts = payload.split('.')
    if len(parts) != 3:
        raise ValueError("Invalid payload format")
    
    ciphertext_b64, hash_b64, iv_b64 = parts
    ciphertext = base64.b64decode(ciphertext_b64)
    hash = base64.b64decode(hash_b64)
    iv = base64.b64decode(iv_b64)

    #check the hashes
    if not verify_hash(ciphertext_b64, hash, hash_key):
       raise ValueError("HASH verification failed")
    
    return float(aes_decrypt(key, iv, ciphertext))

def prepare_payload(threshold):
    key = KEYS["server"]["encrypt"]
    hash_key = KEYS["server"]["hash"]
    iv = get_random_bytes(BLOCK_SIZE)
    #encryption
    ciphertext = aes_encrypt(key, iv, str(threshold))  
    #base64 encoding
    ciphertext_b64 = base64.b64encode(ciphertext).decode('utf-8')

    #hashing
    hash = compute_hash(ciphertext_b64, hash_key)
    hash_b64 = base64.b64encode(hash).decode('utf-8')

    #initial vector encoding
    iv_b64 = base64.b64encode(iv).decode('utf-8')

    return f"{ciphertext_b64}.{hash_b64}.{iv_b64}"

