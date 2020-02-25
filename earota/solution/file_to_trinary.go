package main

import (
	"fmt"
	"os"
	"io/ioutil"
	"math/big"
)


func main() {

	fbuf, err := ioutil.ReadFile("sf_postcard_trinary_flag.png")

	if err != nil {
		fmt.Fprintf(os.Stderr, "Unable to read flag: %s\n", err.Error())
		os.Exit(1)
	}

	fmt.Fprint(os.Stderr, "Converting file to big.Int\n")
	fileint := new(big.Int)
	fileint.SetBytes(fbuf)

	fmt.Fprint(os.Stderr, "Converting big.Int into trinary\n")
	fmt.Println(fileint.Text(3))
}
