#!/usr/bin/env python3

import hashlib
import hmac
from Crypto.Cipher import AES


def hmac_sha256(key, msg):
    return hmac.new(key, msg, hashlib.sha256).digest()


def validate_key_memory(s):
    if len(s) != 3*32:
        return None
    base_key = s[:32]
    crypt_key = s[32:64]
    mac_key = s[64:]
    if hmac_sha256(base_key, b"crypt_key") != crypt_key:
        return None
    if hmac_sha256(base_key, b"mac_key") != mac_key:
        return None
    return (base_key, crypt_key, mac_key)


def keyhunt(buf):
    for i in range(0, len(buf), 8):
        rv = validate_key_memory(buf[i:i+96])
        if rv:
            return rv


def decrypt(buf, key, mac_key):
    sig = buf[-32:]
    buf = buf[:-32]
    expected = hmac_sha256(mac_key, buf)
    if expected != sig:
        print("Sig does not match!")
        return
    cipher = AES.new(key, AES.MODE_CBC, bytes(16))
    ptext = cipher.decrypt(buf)
    return ptext[:-ptext[-1]]


def main():
    with open("../distfiles/core", "rb") as fp:
        buf = fp.read()
    key_data = keyhunt(buf)
    if not key_data:
        print("No key found!!!")
        return
    print("Found key candidate:")
    print("Base key: %s" % key_data[0].hex())
    print("Crypt key: %s" % key_data[1].hex())
    print("MAC key: %s" % key_data[2].hex())
    with open("../distfiles/flag.txt.enc", "rb") as fp:
        buf = fp.read()
    print("Plaintext:")
    print(decrypt(buf, key_data[1], key_data[2]).decode('utf-8'))



if __name__ == '__main__':
    main()
