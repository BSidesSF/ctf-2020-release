CFLAGS := -std=c11 -Wall -fms-extensions #-DDEBUG -ggdb

kvaas: main.o nodes.o prompt.o strings.o
	gcc $(CFLAGS) -o $@ $^

clean:
	-rm -f *.o kvaas

%.o: %.c nodes.h
	gcc $(CFLAGS) -c -o $@ $<
