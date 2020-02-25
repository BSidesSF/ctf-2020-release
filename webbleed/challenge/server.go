package main

import (
	"fmt"
	"io"
	"io/ioutil"
	"math/rand"
	"net/http"
	"strconv"
)

const (
	flag = "CTF{wat_parsing_is_that}"
)

type srv struct{}

var membuf []byte

func (s *srv) ServeHTTP(w http.ResponseWriter, r *http.Request) {
	if r.Method != http.MethodPost {
		// Sending OK for ingress healthchecks
		http.Error(w, "Method Not Allowed", http.StatusOK)
		return
	}
	clen := 0
	clens := r.Header.Get("Content-Length")
	if v, err := strconv.Atoi(clens); err == nil {
		clen = v
		if clen > 1024*1024 {
			http.Error(w, "Request too large", http.StatusRequestEntityTooLarge)
			return
		}
	}
	buf, err := ioutil.ReadAll(r.Body)
	if err != nil && err != io.ErrUnexpectedEOF {
		fmt.Printf("Error: %s\n", err)
		http.Error(w, "Internal Server Error", http.StatusInternalServerError)
		return
	}
	defer r.Body.Close()
	if clen == 0 {
		clen = len(buf)
	}
	w.Header().Set("Content-type", "application/raw-memory")
	fmt.Printf("Content Length: %d\nLength: %d\n", clen, len(buf))
	outbuf := make([]byte, clen)
	n := copy(outbuf, buf)
	if n < clen {
		start := rand.Intn(len(membuf) - (clen - n))
		copy(outbuf[n:], membuf[start:])
	}
	w.Write(outbuf)
}

func initMemBuf() {
	membuf = make([]byte, 16*1024*1024)
	if n, err := rand.Read(membuf); err != nil {
		panic(err)
	} else if n != len(membuf) {
		panic(fmt.Errorf("Expected %d bytes, got %d", len(membuf), n))
	}
	// Insert flag in a few places
	flagBytes := []byte(flag)
	for i := 0; i < 16; i++ {
		start := rand.Intn(len(membuf) - len(flagBytes))
		copy(membuf[start:], flagBytes)
	}
}

func main() {
	initMemBuf()
	http.Handle("/", &srv{})
	http.ListenAndServe(":8888", nil)
}
