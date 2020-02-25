import base64
import gzip
import urllib.parse
import sys

MORSE_CODE_DICT = {'A': '.-', 'B': '-...',
                   'C': '-.-.', 'D': '-..', 'E': '.',
                   'F': '..-.', 'G': '--.', 'H': '....',
                   'I': '..', 'J': '.---', 'K': '-.-',
                   'L': '.-..', 'M': '--', 'N': '-.',
                   'O': '---', 'P': '.--.', 'Q': '--.-',
                   'R': '.-.', 'S': '...', 'T': '-',
                   'U': '..-', 'V': '...-', 'W': '.--',
                   'X': '-..-', 'Y': '-.--', 'Z': '--..',
                   '1': '.----', '2': '..---', '3': '...--',
                   '4': '....-', '5': '.....', '6': '-....',
                   '7': '--...', '8': '---..', '9': '----.',
                   '0': '-----', ', ': '--..--', '.': '.-.-.-',
                   '?': '..--..', '/': '-..-.', '-': '-....-',
                   '(': '-.--.', ')': '-.--.-', '=': '-...-'}


def morse_encode(s):
    return (' '.join(MORSE_CODE_DICT[chr(c)] for c in s)).encode('utf-8')


def decimal_bytes(s):
    return (b' '.join(b'%d' % c for c in s))


def hex_bytes(s):
    return (b', '.join(b'%02x' % c for c in s))


def oct_bytes(s):
    return (b', '.join(b'o%03o' % c for c in s))


def urlencode(s):
    return urllib.parse.quote_plus(s.decode('utf-8')).encode('utf-8')


def main(argv):
    STACK = [
            base64.b32encode,
            morse_encode,
            base64.b85encode,
            decimal_bytes,
            base64.b16encode,
            base64.b64encode,
            gzip.compress,
            base64.b85encode,
            base64.b64encode,
            base64.b32encode,
            morse_encode,
            base64.b64encode,
            gzip.compress,
            base64.b64encode,
            decimal_bytes,
            base64.b85encode,
            urlencode,
            base64.b64encode,
            hex_bytes,
            base64.b64encode,
            gzip.compress,
            base64.b64encode,
            oct_bytes,
            base64.b16encode,
            gzip.compress,
            base64.b16encode,
            gzip.compress,
            base64.b64encode,
            gzip.compress,
            base64.b16encode,
            base64.b32encode,
            gzip.compress,
            base64.b64encode,
            gzip.compress,
            base64.b16encode,
            gzip.compress,
            base64.b64encode,
            base64.b64encode,
            base64.b64encode,
            ]
    with open(argv[1], 'rb') as fp:
        buf = fp.read().strip()
    for s in STACK:
        sys.stderr.write('{}\n'.format(s))
        buf = s(buf)
    print(buf)


if __name__ == '__main__':
    main(sys.argv)
