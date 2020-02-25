package main

import (
	"fmt"
	"os"
	"io/ioutil"
	"math/big"
)


func main() {

	fbuf, err := ioutil.ReadFile("small_btern.txt")
	//fbuf, err := ioutil.ReadFile("test_ternary.txt")

	if err != nil {
		fmt.Fprintf(os.Stderr, "Unable to read flag: %s\n", err.Error())
		os.Exit(1)
	}

	tslice := make([]byte, len(fbuf))

	fmt.Fprint(os.Stderr, "Adding 1s without carry\n")
	dig := 0
	for i, v := range(fbuf) {
		if v == 0x30 {
			tslice[i] = 0x31
			dig += 1
		} else if v == 0x31 {
			tslice[i] = 0x32
			dig += 1
		} else if v == 0x32 {
			tslice[i] = 0x30
			dig += 1
		} else {
			//fmt.Fprintln(os.Stderr, "Got a non [012] digit")
			fmt.Fprintln(os.Stderr, "Skipping addition for non [123] char")
			tslice[i] = v
			//os.Exit(1)
		}
	}

	fmt.Fprint(os.Stderr, "Converting trinary to string\n")
	tri := string(tslice)

	//fmt.Fprint(os.Stderr, "Got post-1+ string: %s\n", tri)

	fmt.Fprint(os.Stderr, "Trinary file to big.Int\n")
	fileint := new(big.Int)
	fileint.SetString(tri, 3)

	// The 1 slice needs to skip all the initial 0 digits
	start := 0
	for _, v := range(tslice) {
		if v == 0x30 {
			start += 1
		} else {
			break
		}
	}

	oneslice := make([]byte, dig - start)

	for i, _ := range(oneslice) {
		oneslice[i] = 0x31 // "1"
	}

	fmt.Fprint(os.Stderr, "Getting a trinary all 1s int of same length\n")
	oneint := new(big.Int)
	oneint.SetString(string(oneslice), 3)

	//fmt.Fprintf(os.Stderr, "Subtracting all 1s int: %s\n", string(oneslice))
	fmt.Fprint(os.Stderr, "Subtracting all 1s int\n")
	fileint.Sub(fileint, oneint)

	fmt.Fprint(os.Stderr, "Writing file\n")
	err = ioutil.WriteFile("out.bin", fileint.Bytes(), 0644)

}
