package main

import (
	"bytes"
	"crypto/rand"
	"crypto/sha256"
	"encoding/binary"
	"fmt"
	"io/ioutil"
	"net/http"
	"net/url"
	"os"
	"runtime/pprof"
	"sync"
)

const (
	HASH_OFFSET = 16
	URL         = "https://hashpop-b0263f3c.challenges.bsidessf.net/cgi-bin/hashpop"
	WORKERS     = 2
)

var (
	targets     = []uint32{0x0804ab40, 0x0804ac60, 0x0804abd0}
	targetBytes = make([][]byte, len(targets))
)

func main() {
	f, err := os.Create("solution.profile")
	if err != nil {
		panic(err)
	}
	pprof.StartCPUProfile(f)
	defer pprof.StopCPUProfile()
	convertTargets()
	ch := make(chan interface{})
	wg := &sync.WaitGroup{}
	keepGoing := func() bool {
		select {
		case <-ch:
			return false
		default:
			return true
		}
	}
	wg.Add(WORKERS)
	for i := 0; i < WORKERS; i++ {
		go func() {
			defer wg.Done()
			srcBuf := make([]byte, 32)
			dstBuf := make([]byte, 0, sha256.Size)
			if _, err := rand.Read(srcBuf); err != nil {
				panic(err)
			}
			h := sha256.New()
			for keepGoing() {
				// Done in chunks to avoid overhead
				for i := 0; i < 1024; i++ {
					h.Reset()
					h.Write(srcBuf)
					dstBuf = h.Sum(dstBuf[:0])
					if hashMatches(dstBuf) {
						close(ch)
						fmt.Printf("Got: %s\n", sendTestCase(srcBuf))
						fmt.Printf("src: %v\n", srcBuf)
						return
					}
					srcBuf, dstBuf = dstBuf, srcBuf
				}
			}
		}()
	}
	wg.Wait()
}

func sendTestCase(buf []byte) string {
	vals := make(url.Values)
	vals.Set("input", string(buf))
	vals.Set("hash", "sha256")
	if resp, err := http.PostForm(URL, vals); err != nil {
		fmt.Printf("Error: %s", err)
		return ""
	} else {
		defer resp.Body.Close()
		if body, err := ioutil.ReadAll(resp.Body); err != nil {
			fmt.Printf("Error: %s", err)
			return ""
		} else {
			return string(body)
		}
	}
}

func convertTargets() {
	for i, t := range targets {
		b := make([]byte, 4)
		binary.LittleEndian.PutUint32(b, t)
		targetBytes[i] = b
	}
}

func hashMatches(buf []byte) bool {
	sub := buf[HASH_OFFSET : HASH_OFFSET+len(targetBytes[0])]
	for _, v := range targetBytes {
		if bytes.Compare(v, sub) == 0 {
			return true
		}
	}
	return false
}
