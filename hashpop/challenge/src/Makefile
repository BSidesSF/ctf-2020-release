CC := /usr/bin/i686-linux-gnu-gcc
CFLAGS := -m32 -std=c99 -Wall -O2 -funroll-loops -no-pie
LIBS := -lcrypto

hashpop: base64.o hashpop.o urldecode.o
	$(CC) $(CFLAGS) $^ $(LIBS) -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f *.o hashpop

packages:
	sudo apt -y install libssl-dev:i386 libgcc-8-dev:i386 gcc-i686-linux-gnu
