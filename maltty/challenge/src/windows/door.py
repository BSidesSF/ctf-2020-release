from Crypto.Cipher import AES
import hashlib


KEY = (b"\x7c\x58\x4d\x1f\x8e\x3c\x63\xe4\x07\xf6\x18\x2c"
       b"\x56\xdc\x5f\x3e\xd7\x09\x1a\x5b\x8f\x01\x8b\x4a"
       b"\x75\xc1\xfa\xaa\xb9\xf4\x80\xf9")
IV = (b"\x5d\xa1\x4f\xf2\x3a\x04\xaa\xd0"
      b"\x85\x64\x25\xa1\x9e\x97\x93\x6e")

PTEXT = ("GET /%s HTTP/1.1\r\n"
         "Host: bsidessf.pwned\r\n"
         "X-Flag: CTF{notJustCCleaner}\r\n\r\n")

HOST = "localhost:9999"

TMPL = (
"""#include <sys/types.h>

#define POST_LEN %d

char g_secbuf[] = {%s};
""")


def encrypt_all(k, iv, val):
    ciph = AES.new(k, AES.MODE_CBC, iv)
    return ciph.encrypt(val)


def pad_s(s):
    if isinstance(s, str):
        s = s.encode("utf-8")
    while len(s) % 16:
        s += b"\x00"
    return s


def sha256(s):
    if isinstance(s, str):
        s = s.encode("utf-8")
    return hashlib.sha256(s).digest()


def main():
    s = HOST + "\x00" + PTEXT
    if isinstance(s, str):
        s = s.encode("utf-8")
    padded = pad_s(s)
    ptext = sha256(padded) + padded
    encrypted = encrypt_all(KEY, IV, ptext)
    encrypted_s = ', '.join(('0x%02x' % (c)) for c in encrypted)
    print(TMPL % (len(encrypted), encrypted_s))


if __name__ == '__main__':
    main()
