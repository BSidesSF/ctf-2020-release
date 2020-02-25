crypto: crypto.c
	gcc -Wall -o crypto crypto.c -lcrypto

.PRECIOUS: test.txt.enc flag.txt.enc

test.txt.enc: crypto
	cp crypto.c test.txt
	-./crypto b5bb9d8014a0f9b1d61e21e796d78dccdf1352f23cd32812f4850b878ae4944c test.txt

flag.txt.enc: crypto flag.txt
	rm -f ./core
	-./crypto 718cc27c39d39e36c986d1eac98dd19cc67262bad8398d4bed4d787e85a8e9f2 flag.txt
	cp ./crypto ./core ./flag.txt.enc ../../distfiles/
	rm -f ./core
