#!/usr/bin/python

import math

tri_name = 'trinary.txt'

print('reading data')
with open(tri_name, 'r') as f:
    data = f.read()

data.rstrip()

print('converting to int from base 3')
i = int(data, 3)

print('converting int to bytes')
s = i.to_bytes(math.ceil(i.bit_length() / 8), byteorder='big')

print('writing file')
with open('test.bin', 'wb') as f:
    f.write(s)

