import random

PSK = "CTF{good_work_senpai}"
PASSWD = "eefi0shush9och5Ama"

TMPL = """
#define _SECRET_HOLDER_ 1
#include "locky_secrets.h"

static const int ekey = %(ekey)d;
static const int _aa_len = %(psk_len)d;
static const int _bb_len = %(passwd_len)d;

static const char _aa[] = {%(psk)s};
static const char _bb[] = {%(passwd)s};
const void *SECRETS[] = {
    (void *)&ekey,
    (void *)_aa,
    (void *)&_aa_len,
    (void *)_bb,
    (void *)&_bb_len
    };
"""


def enc_str(a, k):
    output = []
    for c in a:
        output.append(ord(c) ^ k)
        k = (k + 1) & 0xFF
    return ', '.join('0x%02x' % c for c in output)


def main():
    key = random.randint(1, 255)
    data = {
            'ekey': key,
            'psk_len': len(PSK),
            'passwd_len': len(PASSWD),
            'psk': enc_str(PSK, key),
            'passwd': enc_str(PASSWD, key),
            }
    print(TMPL % data)


if __name__ == '__main__':
    main()
