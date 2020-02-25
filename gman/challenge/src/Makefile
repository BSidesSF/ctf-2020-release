all: gman

gman: gman.c password.c
	gcc -o gman -Wl,--section-start=.data=0x08040800 -Wall -m32 -no-pie -O0 gman.c password.c -lcurses -ltinfo

clean:
	rm -f gman *.o
