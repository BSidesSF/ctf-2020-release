echo -en 'POST / HTTP/1.1\nHost: fucker\nContent-Length: 256000\n\nfoo\n' | nc -N localhost 8888 | hexdump -C
