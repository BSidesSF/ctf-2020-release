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

	pslice := make([]byte, len(fbuf))
	nslice := make([]byte, len(fbuf))

	fmt.Fprint(os.Stderr, "Reading 0, 1, 2 into positive and negative nums\n")
	dig := 0
	for i, v := range(fbuf) {
		if v == 0x30 {
			pslice[i] = 0x30
			nslice[i] = 0x30
			dig += 1
		} else if v == 0x31 {
			pslice[i] = 0x31
			nslice[i] = 0x30
			dig += 1
		} else if v == 0x32 {
			pslice[i] = 0x30
			nslice[i] = 0x31
			dig += 1
		} else {
			//fmt.Fprintln(os.Stderr, "Got a non [012] digit")
			fmt.Fprintln(os.Stderr, "Skipping addition for non [123] char")
			pslice[i] = v
			nslice[i] = v
			//os.Exit(1)
		}
	}

	fmt.Fprint(os.Stderr, "Converting trinary to strings\n")
	trip := string(pslice)
	trin := string(nslice)

	//fmt.Fprint(os.Stderr, "Got post-1+ string: %s\n", tri)

	fmt.Fprint(os.Stderr, "Positive trinary file to big.Int\n")
	pint := new(big.Int)
	pint.SetString(trip, 3)

	fmt.Fprint(os.Stderr, "Negative trinary file to big.Int\n")
	nint := new(big.Int)
	nint.SetString(trin, 3)

	fileint := new(big.Int)

	fmt.Fprint(os.Stderr, "Subtracting negative int from positive int\n")
	fileint.Sub(pint, nint)

	fmt.Fprint(os.Stderr, "Writing file\n")
	err = ioutil.WriteFile("out.bin", fileint.Bytes(), 0644)

}
