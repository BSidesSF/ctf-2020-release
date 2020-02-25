package main

import (
	"fmt"
	"os"
	"io/ioutil"
	"math/big"
)


func main() {

	//fbuf, err := ioutil.ReadFile("sf_postcard_trinary_flag.png")
	fbuf, err := ioutil.ReadFile("testin.bin")

	if err != nil {
		fmt.Fprintf(os.Stderr, "Unable to read flag: %s\n", err.Error())
		os.Exit(1)
	}

	fmt.Fprint(os.Stderr, "Converting file to big.Int\n")
	fileint := new(big.Int)
	fileint.SetBytes(fbuf)

	fmt.Fprint(os.Stderr, "Converting big.Int into trinary\n")
	trislice := []byte(fileint.Text(3))

	oneslice := make([]byte, len(trislice))

	for i, _ := range(oneslice) {
		oneslice[i] = 0x31 // "1"
	}

	fmt.Fprint(os.Stderr, "Getting a trinary all 1s int of same length\n")
	oneint := new(big.Int)
	oneint.SetString(string(oneslice), 3)

	fmt.Fprint(os.Stderr, "Adding all 1s int\n")
	fileint.Add(fileint, oneint)

	newslice :=  []byte(fileint.Text(3))
	btslice := make([]byte, len(newslice))

	fmt.Fprint(os.Stderr, "Subtracting 1s without carry\n")
	for i, v := range(newslice) {
		if v == 0x30 {
			btslice[i] = 0x32
		} else if v == 0x31 {
			btslice[i] = 0x30
		} else if v == 0x32 {
			btslice[i] = 0x31
		} else {
			fmt.Fprintln(os.Stderr, "Got a non [012] digit")
			os.Exit(1)
		}
	}

	fmt.Println(string(btslice))
}
