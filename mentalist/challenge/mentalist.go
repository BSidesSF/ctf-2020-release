package main

import (
	"bufio"
	"fmt"
	"io/ioutil"
	"math/big"
	"os"
	"path"
	"syscall"
	cryptor "crypto/rand"
)

var flag string

var banner = `
                                         ____
                                       .'* *.'
                                    __/_*_*(_
                                   / _______ \
                                  _\_)/___\(_/_
                                 / _((\- -/))_ \
                                 \ \())(-)(()/ /
                                  ' \(((()))/ '
                                 / ' \)).))/ ' \
                                / _ \ - | - /_  \
                               (   ( .;''';. .'  )
                               _\"__ / HA )\ __"/_
                                 \/  \  CK /  \/
                                  .'  '...' ' )
                                   / /  |  \ \
                                  / .   .   . \
                                 /   .     .   \
                                /   /   |   \   \
                              .'   /    .    '.  '.
                          _.-'    /     ..     '-. '-._
                      _.-'       |      ...       '-.  '-.
                     (___________\____......'________)____)
 _  _ ____ __   ___ __  _  _ ___    ___ _  _  __  ___ ___ __ _    __  __ _ ___
/ )( (  __|  ) / __)  \( \/ | __)  / __) )( \/  \/ __| __|  ( \  /  \(  ( ( __)
\ /\ /) _)/ (_( (_(  O ) \/ \)_)  ( (__) __ (  O )__ \)_)/    / (  O )    /)_)
(_/\_|____)___/\___)__/\_)(_(___)  \___)_)(_/\__/(___(___)_)__)  \__/\_)__|___)

`

var primes = [...]*big.Int{big.NewInt(2), big.NewInt(3), big.NewInt(5), big.NewInt(7), big.NewInt(11), big.NewInt(13)}

var fail = [...]string{
	"Actually I was thinking of %s, try again",
	"No I'm sorry, I was thinking of %s",
	"Hmmm no. My number was %s, are you sure you're okay?",
	"I'm getting worried. I was thinking of %s; you're not doing so well.",
	"I grow tired of your failures. My number was %s",
	"Nope. %s Perhaps you aren't the one I was waiting for?",
	"WRONG! It was %s",
	"My patience thins... %s was my number",
	"You're getting on my nerves. It was %s",
	"I'm only going to give you one more chance. I was thinking of %s"}



var PA, PM, PC, PS *big.Int

var UB = big.NewInt(1000000000000000)
var LB = big.NewInt(1000000000)

func main() {
	startup()

	fbuf, err := ioutil.ReadFile("./flag.txt")

	if err != nil {
		fmt.Fprintf(os.Stderr, "Unable to read flag: %s\n", err.Error())
		os.Exit(1)
	}
	flag = string(fbuf)

	r := big.NewInt(0)
	r.Sub(UB, LB)
	r.Div(r, primes[len(primes) - 1])
	t, err := cryptor.Int(cryptor.Reader, r)
	t.Add(t, LB) // Our target for the modulus

	PM = big.NewInt(1)
	for PM.Cmp(t) < 0 {
		iB, _ := cryptor.Int(cryptor.Reader, big.NewInt(int64(len(primes))))
		PM.Mul(PM, primes[iB.Uint64()])
	}
	//fmt.Fprintf(os.Stderr, "Generated PM: %s\n", PM.String())


	PA = big.NewInt(4)
	for _, v := range primes {
		PA.Mul(PA, v)
	}
	l := new(big.Int)
	l.Set(PM)
	l.Sub(l, big.NewInt(1))
	l.Div(l, PA)
	l.Sub(l, big.NewInt(1))
	lF, _ := cryptor.Int(cryptor.Reader, l)
	lF.Add(lF, big.NewInt(1))
	PA.Mul(PA, lF)
	PA.Add(PA, big.NewInt(1))
	//fmt.Fprintf(os.Stderr, "Generated PA: %s\n", PA.String())

	lC := new(big.Int)
	lC.Set(PM)
	lC.Sub(lC, big.NewInt(1))
	PC, _ = cryptor.Int(cryptor.Reader, lC)
	PC.Add(PC, big.NewInt(1))
	pGCD := new(big.Int)
	pGCD.GCD(nil, nil, PC, PM)
	PC.Div(PC, pGCD)
	//fmt.Fprintf(os.Stderr, "Generated PC: %s\n", PC.String())

	PS, _ = cryptor.Int(cryptor.Reader, PM)
	//fmt.Fprintf(os.Stderr, "Generated PS: %s\n", PS.String())

	//negone := big.NewInt(-1)

	input := bufio.NewScanner(os.Stdin)

	fmt.Fprint(os.Stdout, "Welcome Chosen One! I have been waiting for you...\n")
	fmt.Fprint(os.Stdout, "The legend fortold of one that could read minds.\n")
	fmt.Fprint(os.Stdout, "If you can read my mind I will reveal my great knowledge.\n")

	// x := new(big.Int)
	// x.Set(PS)
	// for i := 0; i < 20; i++ {
	// x = LCG(PA, PC, PM, x)
	// fmt.Fprintf(os.Stdout, "%s\n", x.String())
	// }


	failc := 0
	passc := 0
	exit := false

	curg := PS
	for !exit {
		fmt.Fprint(os.Stdout, "\nWhat number am I thinking of? ")
		ok := input.Scan()
		if !ok {
			fmt.Fprintln(os.Stdout, "\nIf you are going to waste my time we're through here.")
			break
		}

		text := input.Text()

		if len(text) == 0 {
			fmt.Fprintln(os.Stdout, "I'm sorry but that's not a guess.")
			continue
		}

		g := new(big.Int)
		g, ok = g.SetString(text, 10)

		if !ok || g == nil {
			fmt.Fprintln(os.Stdout, "That was gibberish. Goodbye!")
			break
		}

		if g.Cmp(big.NewInt(0)) < 0 {
			fmt.Fprintln(os.Stdout, "I only think positively and you should too. Come back when you're ready.")
			break
		}

		if g.Cmp(UB) > 0 {
			fmt.Fprintln(os.Stdout, "What sort of cheater do you take me for?")
			fmt.Fprintf(os.Stdout, "I never think of numbers greater than %s\n", UB.String())
			break
		}

		curg = LCG(PA, PC, PM, curg)
		if g.Cmp(curg) != 0 {
			if failc < len(fail) {
				if passc > 0 {
					fmt.Fprintln(os.Stdout, "It seems you got lucky. Too bad for me.")
					break
				} else {
					fmt.Fprintf(os.Stdout, fail[failc], curg.String())
					failc++
				}
			} else {
				fmt.Fprintln(os.Stdout, "I see now that you aren't who I was looking for.")
				fmt.Fprintf(os.Stdout, "It's too late now but I was thinking of %s\n", curg.String())
				fmt.Fprintln(os.Stdout, "In case you were wondering how I was thinking of these numbers,")
				fmt.Fprintf(os.Stdout, "they were for the form x_n+1 = x_n * %s + %s %% %s\n",
					PA.String(), PC.String(), PM.String())
				fmt.Fprintf(os.Stdout, "And my initial seed x_0 was %s\n", PS.String());
				fmt.Fprintln(os.Stdout, "With this you can verify that I wasn't cheating.")
				fmt.Fprintln(os.Stdout, "Good luck in your future endeavors!")
				break
			}
		} else {
			if passc == 0 {
				fmt.Fprint(os.Stdout, "Incredible! I WAS thinking of that number! But can you do it again?")
				passc++
			} else {
				fmt.Fprintln(os.Stdout, "You really are the one that was foretold. Please accept this knowldege:")
				fmt.Fprintln(os.Stdout, flag)
				break
			}
		}
	}
}

func next_prime(n *big.Int) *big.Int {

	pp := new(big.Int)
	one := big.NewInt(1)

	pp.Set(n)
	for {
		if !pp.ProbablyPrime(20) {
			pp.Add(pp, one)
		} else {
			break
		}
	}

	return pp
}

func startup() {
	fmt.Fprint(os.Stdout, banner)
	changeBinDir()
	limitTime(5)
}

// Change to working directory
func changeBinDir() {
	// read /proc/self/exe
	if dest, err := os.Readlink("/proc/self/exe"); err != nil {
		fmt.Fprintf(os.Stderr, "Error reading link: %s\n", err)
		return
	} else {
		dest = path.Dir(dest)
		if err := os.Chdir(dest); err != nil {
			fmt.Fprintf(os.Stderr, "Error changing directory: %s\n", err)
		}
	}
}

// Limit CPU time to certain number of seconds
func limitTime(secs int) {
	lims := &syscall.Rlimit{
		Cur: uint64(secs),
		Max: uint64(secs),
	}
	if err := syscall.Setrlimit(syscall.RLIMIT_CPU, lims); err != nil {
		if inner_err := syscall.Getrlimit(syscall.RLIMIT_CPU, lims); inner_err != nil {
			fmt.Fprintf(os.Stderr, "Error getting limits: %s\n", inner_err)
		} else {
			if lims.Cur > 0 {
				// A limit was set elsewhere, we'll live with it
				return
			}
		}
		fmt.Fprintf(os.Stderr, "Error setting limits: %s", err)
		os.Exit(-1)
	}
}


func LCG(a, c, m, x *big.Int) *big.Int {
	nx := new(big.Int)
	nx.Mul(x, a)
	nx.Add(nx, c)
	nx.Mod(nx, m)

	return nx
}
