adder: adder.o adder_asm.o
	gcc -o $@ -masm=intel -fstack-protector-all -Wall $^

clean:
	rm -f adder *.o

adder.o: adder.c
	gcc -c -o $@ -masm=intel -fstack-protector-all -Wall $^

adder_asm.o: adder_asm.S
	gcc -c -o $@ -masm=intel -fstack-protector-all -Wall $^
