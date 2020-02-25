ruler: ruler.c
	gcc -no-pie -o ruler ruler.c -Wl,-Bstatic -lcrypto -lssl -Wl,-Bdynamic -ldl -lpthread
	strip ruler

ruler.dbg: ruler.c
	gcc -ggdb -no-pie -o ruler.dbg ruler.c -Wl,-Bstatic -lcrypto -lssl -Wl,-Bdynamic -ldl -lpthread
