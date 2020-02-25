setrand(extern("echo 0x`od -An -N8 -t x8 /dev/urandom | awk '{print $1}'`"));
d = randomprime(2^64);
a = randomprime(2^32);
b = randomprime(2^32);
while(b == a, b = randomprime(2^32););
s = random(2^32) + 2^32;
e = ellinit([a, b], d);
p = random(e);
r = ellmul(e, p, s);
printf("d=%d; a=%d; b=%d; s=%d; px=%d; py=%d; rx=%d; ry=%d;\n", d, a, b, s, lift(p[1]), lift(p[2]), lift(r[1]), lift(r[2]));
\q
