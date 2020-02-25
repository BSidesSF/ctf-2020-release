package main

import (
	"fmt"
	"os"
	"io/ioutil"
	"math/big"
)


func main() {

	fbuf, err := ioutil.ReadFile("trinary.txt")

	if err != nil {
		fmt.Fprintf(os.Stderr, "Unable to read flag: %s\n", err.Error())
		os.Exit(1)
	}

	fmt.Fprint(os.Stderr, "Converting trinary to string\n")
	tri := string(fbuf)

	fmt.Fprint(os.Stderr, "Trinary file to big.Int\n")
	fileint := new(big.Int)
	fileint.SetString(tri, 3)

	fmt.Fprint(os.Stderr, "Writing file\n")
	err = ioutil.WriteFile("out.bin", fileint.Bytes(), 0644)

}
