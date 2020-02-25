import collections
import sys
import struct


def main():
    if len(sys.argv) > 1:
        fname = sys.argv[1]
    else:
        fname = "/dev/stdin"
    with open(fname, "rb") as fp:
        buf = fp.read()
    res = decode_buf(buf)
    with open('/tmp/buf', 'wb') as fp:
        fp.write(res)
    try:
        print(res.decode('utf-8'))
    except:
        pass


def decode_buf(buf):
    intcounter = lambda: collections.defaultdict(int)
    char_counter = collections.defaultdict(intcounter)
    msglen = struct.unpack('>I', buf[:4])[0]
    buf = buf[4:]
    char_len = 17
    lookup = {}
    for i in range(len(buf)//char_len):
        chunk = buf[i*char_len:(i+1)*char_len]
        c_val, hash_val = chunk[0], chunk[1:]
        char_counter[c_val][hash_val] += 1
    for c_val, ctr in char_counter.items():
        max_ct = 0
        max_h = ""
        for _, count in ctr.items():
            if count > max_ct:
                max_ct = count
        if max_ct == 1:
            print("Character %d (%c) is underspecified." % (c_val, c_val))
            continue
        for h, count in ctr.items():
            if count != max_ct:
                continue
            if max_h != "":
                print("Character %d (%c) is under-specified." % (c_val, c_val))
            lookup[h] = c_val
    results = []
    print(lookup)
    found = 0
    for ck in range(msglen):
        cklen = len(buf)//msglen
        chunk = buf[ck*cklen:(ck+1)*cklen]
        for p in range(cklen//char_len):
            piece = chunk[p*char_len:(p+1)*char_len]
            char, h = piece[0], piece[1:]
            if h in lookup:
                results.append(char)
                found += 1
                break
    print('found %d out of %d' % (found, msglen))
    return bytes(results)


if __name__ == '__main__':
    main()
