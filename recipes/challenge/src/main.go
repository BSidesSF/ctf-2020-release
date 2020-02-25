package main

import (
	"log"
	"net/http"
)

const (
	ListenAddress = "0.0.0.0:8080"
)

func main() {
	log.Printf("Starting serving on %s", ListenAddress)
	log.Fatal(http.ListenAndServe(ListenAddress, nil))
}
