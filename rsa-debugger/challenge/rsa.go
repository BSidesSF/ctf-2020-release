package main

import (
	"fmt"
	"io/ioutil"
	"bufio"
	"strings"
	"os"
	"path"
	"syscall"
	"math/big"
	//"math"
	//"crypto/aes"
	//"crypto/cipher"
	//"encoding/binary"
	//"encoding/hex"
	cryptor "crypto/rand"
	//mathr "math/rand"
	"time"
)

type Key struct {
	p big.Int
	q big.Int
	n big.Int
	e big.Int
	d big.Int
}

var CurKey Key
var CurMsg big.Int

var TargMsg big.Int
var BaseTarg big.Int

var flag string

var banner = `
              ______               K         /$$$$$$$
           ,'"       "-._         C         | $$__  $$
         ,'              "-._ _  A          | $$  \ $$
         ;              __,-'/  H           | $$$$$$$/
        ;|           ,-' _,'"'._,'          | $$__  $$
        |:            _,'      |\ '.        | $$  \ $$
        : \       _,-'         | \  '.      | $$  | $$
         \ \   ,-'             |  \   \     |__/  |__/ emote
          \ '.         .-.     |       \
           \  \         "      |        :               /$$$$$$
            '. '.              |        |              /$$__  $$
              '. "-._          |        ;             | $$  \__/
              / |'._ '-._      /       /              |  $$$$$$
             /  | \ '._   "-.___    _,'                \____  $$
            /   |  \_.-"-.___   """"                   /$$  \ $$
            \   :            /"""                     |  $$$$$$/
             '._\_       __.'_                         \______/  atellite
        __,--''_ ' "--'''' \_  '-._
  __,--'     .' /_  |   __. '-.    '-._                           /$$$$$$
 /            '.  '-.-''  __,-'     _,-'                         /$$__  $$
  '.            '.   _,-'"      _,-'                            | $$  \ $$
    '.            ''"       _,-'                                | $$$$$$$$
      '.                _,-'                                    | $$__  $$
        '.          _,-'                                        | $$  | $$
          '.   __,'"                                            | $$  | $$
            ''"                                                 |__/  |__/ ttack
`


func main() {
	startup()

	fbuf, err := ioutil.ReadFile("./flag.txt");

	if err != nil {
		fmt.Fprintf(os.Stderr, "Unable to read flag: %s\n", err.Error())
		os.Exit(1)
	}
	flag = string(fbuf)

	negone := big.NewInt(-1)

	CurMsg.Set(negone)
	TargMsg.Set(negone)

	// perl -e 'print unpack("H*", "\x01SET DEBUG\x02LOAD KEY\x03DECRYPT MSG\x04EXEC GETFLAG\x00\x00\x00\x00\x00"), "\n";'
	BaseTarg.SetString("52218557622655182058721298410128724497736237107858961398752582948746717509543923532995392133766377362569696093667328", 10)

	CurKey.p.Set(negone)
	CurKey.q.Set(negone)
	CurKey.n.Set(negone)
	CurKey.e.Set(negone)
	CurKey.d.Set(negone)

	input := bufio.NewScanner(os.Stdin);

	fmt.Fprint(os.Stdout, "\nWelcome to the Remote Satellite Attack Debugger!\n")
	fmt.Fprint(os.Stdout, "\nTry \"help\" for a list of commands\n")

	exit := false

	for !exit {
		fmt.Fprint(os.Stdout, "\nRSA debugger> ")
		ok := input.Scan()
		if !ok {
			fmt.Fprintln(os.Stdout, "")
			break
		}

		text := input.Text()

		if len(text) == 0 {
			continue
		}
		//fmt.Fprintf(os.Stdout, "Got command: %s\n", text)

		tokens := strings.Split(text, " ")

		switch tokens[0] {

		case "help":
			print_help()

		case "h":
			print_help()

		case "?":
			print_help()

		case "background":
			print_background()

		case "printkey":
			print_key()

		case "resetkey":
			reset_key()
			fmt.Fprintf(os.Stdout, "Key parameters reset.\n")

		case "holdmsg":
			hold_msg()

		case "printmsg":
			print_msg()

		case "printtarget":
			print_target()

		case "setp":
			set_p(tokens)

		case "setq":
			set_q(tokens)

		case "sete":
			set_e(tokens)

		case "testdecrypt":
			test_decrypt_msg()

		case "attack":
			attack()
			fmt.Fprintln(os.Stdout, "")
			os.Exit(0)

		case "exit":
			exit = true

		case "quit":
			exit = true

		default:
			fmt.Fprintf(os.Stdout, "RSA debugger: %s: command not found. Try \"help\" for a list of commands.", tokens[0])
		}

	// 	commands.ParseAndExecute(sess, cmd)
	}
}


func print_help() {
	fmt.Fprintln(os.Stdout, "Remote Satellite Attack Debugger help:")
	fmt.Fprintln(os.Stdout, "")
	fmt.Fprintln(os.Stdout, "Commands:")
	fmt.Fprintln(os.Stdout, "    help            # Prints this help")
	fmt.Fprintln(os.Stdout, "    background      # Explain how the attack works")
	fmt.Fprintln(os.Stdout, "    holdmsg         # Holds a suitable message from being transmitted")
	fmt.Fprintln(os.Stdout, "    printmsg        # Prints the currently held message")
	fmt.Fprintln(os.Stdout, "    printtarget     # Prints the target plaintext for currently held msg")
	fmt.Fprintln(os.Stdout, "    setp <int>      # Set p to the value specified")
	fmt.Fprintln(os.Stdout, "       e.g. setp 127")
	fmt.Fprintln(os.Stdout, "    setq <int>      # Set q to the value specified (p must be set)")
	fmt.Fprintln(os.Stdout, "       e.g. setq 131")
	fmt.Fprintln(os.Stdout, "    sete <int>      # Set e to the value specified (p & q must be set)")
	fmt.Fprintln(os.Stdout, "       e.g. sete 17")
	fmt.Fprintln(os.Stdout, "    printkey        # Prints the current attack key")
	fmt.Fprintln(os.Stdout, "    resetkey        # Clears all the set key parameters")
	fmt.Fprintln(os.Stdout, "    testdecrypt     # Locally decrypts held message with current key")
	fmt.Fprintln(os.Stdout, "    attack          # Send the key and held message to the satellite")
	fmt.Fprintln(os.Stdout, "    exit            # Exit the hacking interface")
}


func print_background() {
	fmt.Fprintln(os.Stdout, "Remote Satellite Attack Debugger background:")
	fmt.Fprintln(os.Stdout, "")
	fmt.Fprintln(os.Stdout, "Our agents were able to obtain a working prototype of one of the SATNET")
	fmt.Fprintln(os.Stdout, "satellites and through extensive reverse engineering uncovered a")
	fmt.Fprintln(os.Stdout, "debugging interface that has not been disabled. We believe we've")
	fmt.Fprintln(os.Stdout, "uncovered a vulnerability that will let us take control of a satellite.")
	fmt.Fprintln(os.Stdout, "If we sent our own messages to the satellite, we'd get caught in the")
	fmt.Fprintln(os.Stdout, "message audit. Instead, we've found a way to intercept and delay messages")
	fmt.Fprintln(os.Stdout, "in transmission. By uploading a new key via the debugging interface we")
	fmt.Fprintln(os.Stdout, "should be able to manipulate how the satellite interprets the message after")
	fmt.Fprintln(os.Stdout, "the message is decrypted.")
	fmt.Fprintln(os.Stdout, "")
	fmt.Fprintln(os.Stdout, "The attack:")
	fmt.Fprintln(os.Stdout, "Using the command `holdmsg` we will begin searching the outbound messages")
	fmt.Fprintln(os.Stdout, "for a suitable message ciphertext. When a message is found, we can derive")
	fmt.Fprintln(os.Stdout, "the plaintext that we need the message to decrypt to. You can see the held")
	fmt.Fprintln(os.Stdout, "message with `printmsg` and the desired plaintext with `printtarget`.")
	fmt.Fprintln(os.Stdout, "")
	fmt.Fprintln(os.Stdout, "The satellite will accept a new private key with only a few basic checks:")
	fmt.Fprintln(os.Stdout, "1) p and q must be primes")
	fmt.Fprintln(os.Stdout, "2) p and q must be co-prime")
	fmt.Fprintln(os.Stdout, "3) e must be co-prime to the Euler totient of n")
	fmt.Fprintln(os.Stdout, "")
	fmt.Fprintln(os.Stdout, "Note that we only send the satellite p, q, and e and it derives n and d.")
	fmt.Fprintln(os.Stdout, "")
	fmt.Fprintln(os.Stdout, "When the right key has been found, use `attack` to upload the new key")
	fmt.Fprintln(os.Stdout, "and release the held message. The satellite will decrypt the message")
	fmt.Fprintln(os.Stdout, "with our provided key. If the resulting plaintext contains the target")
	fmt.Fprintln(os.Stdout, "debugging commands we should gain control of the satellite.")
}


func print_msg() {
	negone := big.NewInt(-1)

	if CurMsg.Cmp(negone) != 0 {
		fmt.Fprintf(os.Stdout, "Held Message: %s\n", CurMsg.Text(10))
	} else {
		fmt.Fprintf(os.Stdout, "Error: no message being held!\n")
	}
}


func print_target() {
	negone := big.NewInt(-1)

	if TargMsg.Cmp(negone) != 0 {
		fmt.Fprintf(os.Stdout, "Target plaintext for held message: %s\n", TargMsg.Text(10))
	} else {
		fmt.Fprintf(os.Stdout, "Error: no target plaintext available!\n")
	}
}


func hold_msg() {

	negone := big.NewInt(-1)

	if CurMsg.Cmp(negone) != 0 {
		fmt.Fprintf(os.Stdout, "Currently held message released.\n")
		CurMsg.Set(negone)
		TargMsg.Set(negone)
	}

	fmt.Fprintf(os.Stdout, "Holding message")

	m, err := cryptor.Prime(cryptor.Reader, 2048)

	if err != nil {
		fmt.Fprintf(os.Stdout, "Error: unable to hold message!\n")
		return
	} else {
		fmt.Fprintf(os.Stdout, ".")

		rb, err := cryptor.Int(cryptor.Reader, big.NewInt(3))
		if err != nil {
			fmt.Fprintf(os.Stdout, "Error: unable to hold message!\n")
			return
		}

		r := rb.Uint64() + 2
		for i := uint64(0); i < r; i++ {
			time.Sleep(1 * time.Second)
			fmt.Fprintf(os.Stdout, ".")
		}
		fmt.Fprintf(os.Stdout, "found a message to hold!\n")
	}

	CurMsg.Set(m)

	rb, err := cryptor.Int(cryptor.Reader, big.NewInt(2147483648))

	if err != nil {
		fmt.Fprintf(os.Stdout, "Error: unable to find suitable target plaintext!\n")
		return
	}

	TargCand := new(big.Int)
	TargCand.Add(&BaseTarg, rb)
	TargMsg.Set(next_prime(TargCand))

	fmt.Fprintf(os.Stdout, "Target plaintext derived.")
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


func decrypt_msg() *big.Int {

	negone := big.NewInt(-1)

	if CurKey.d.Cmp(negone) == 0 {
		fmt.Fprintf(os.Stdout, "Error: key not fully set!")
		return nil
	}

	if CurMsg.Cmp(negone) == 0 {
		fmt.Fprintf(os.Stdout, "Error: no message held for decryption!")
		return nil
	}

	ptext := new(big.Int)
	ptext.Exp(&CurMsg, &(CurKey.d), &(CurKey.n))

	return ptext
}


func test_decrypt_msg() {

	ptext := decrypt_msg()

	if ptext != nil {
		fmt.Fprintf(os.Stdout, "Message decrypted to: %s\n", ptext.Text(10))
	}
}

func attack() {

	negone := big.NewInt(-1)

	if CurKey.d.Cmp(negone) == 0 {
		fmt.Fprintf(os.Stdout, "Error: the satellite rejected our key!")
		return
	}

	if CurMsg.Cmp(negone) == 0 {
		fmt.Fprintf(os.Stdout, "Error: we failed to send a held message!")
		return
	}

	ptext := decrypt_msg()

	if ptext == nil {
		fmt.Fprintf(os.Stdout, "Error: The satellite failed to decrypt the message")
		return
	}

	if TargMsg.Cmp(ptext) != 0 {
		fmt.Fprintf(os.Stdout, "Error: The satellite stopped responding")
	} else {
		fmt.Fprintf(os.Stdout, "Satellite response: %s", flag)
	}
}


func reset_key() {

	negone := big.NewInt(-1)

	CurKey.p.Set(negone)
	CurKey.q.Set(negone)
	CurKey.n.Set(negone)
	CurKey.e.Set(negone)
	CurKey.d.Set(negone)
}


func print_key() {
	negone := big.NewInt(-1)

	fmt.Fprintln(os.Stdout, "Current key parameters:")

	// p
	if CurKey.p.Cmp(negone) != 0 {
		fmt.Fprintf(os.Stdout, " p: %s\n", CurKey.p.Text(10))
	} else {
		fmt.Fprintf(os.Stdout, " p: unset\n")
	}

	// q
	if CurKey.q.Cmp(negone) != 0 {
		fmt.Fprintf(os.Stdout, " q: %s\n", CurKey.q.Text(10))
	} else {
		fmt.Fprintf(os.Stdout, " q: unset\n")
	}

	// n
	if CurKey.n.Cmp(negone) != 0 {
		fmt.Fprintf(os.Stdout, " derived n: %s\n", CurKey.n.Text(10))
	} else {
		fmt.Fprintf(os.Stdout, " derived n: p & q unset\n")
	}

	// e
	if CurKey.e.Cmp(negone) != 0 {
		fmt.Fprintf(os.Stdout, " e: %s\n", CurKey.e.Text(10))
	} else {
		fmt.Fprintf(os.Stdout, " e: unset\n")
	}

	// d
	if CurKey.d.Cmp(negone) != 0 {
		fmt.Fprintf(os.Stdout, " derived d: %s\n", CurKey.d.Text(10))
	} else {
		fmt.Fprintf(os.Stdout, " derived d: e unset\n")
	}
}


func set_p(t []string) {

	negone := big.NewInt(-1)

	if (len(t) != 2) {
		fmt.Fprintf(os.Stdout, "Error: setp must have exactly one argument!\n")
		return
	}

	if CurKey.q.Cmp(negone) != 0 {
		fmt.Fprintf(os.Stdout, "Unsetting q, n, e, d\n")
		CurKey.q.Set(negone)
		CurKey.n.Set(negone)
		CurKey.e.Set(negone)
		CurKey.d.Set(negone)
	}

	p := new(big.Int)
	_, ok := p.SetString(t[1], 10)

	if !ok {
		fmt.Fprintf(os.Stdout, "Error: could not parse numerical argument to setp!\n")
		return
	}

	if !p.ProbablyPrime(20) {
		fmt.Fprintf(os.Stdout, "Error: the argument to setp must be prime!\n")
		return
	}

	CurKey.p.Set(p)
}


func set_q(t []string) {

	negone := big.NewInt(-1)
	one := big.NewInt(1)

	if (len(t) != 2) {
		fmt.Fprintf(os.Stdout, "Error: setq must have exactly one argument!\n")
		return
	}

	if CurKey.p.Cmp(negone) == 0 {
		fmt.Fprintf(os.Stdout, "Error: p must be set first!\n")
		return
	}

	if CurKey.q.Cmp(negone) != 0 {
		fmt.Fprintf(os.Stdout, "Unsetting e, d\n")
		CurKey.e.Set(negone)
		CurKey.d.Set(negone)
	}

	q := new(big.Int)
	_, ok := q.SetString(t[1], 10)

	if !ok {
		fmt.Fprintf(os.Stdout, "Error: could not parse numerical argument to setq!\n")
		return
	}

	if !q.ProbablyPrime(20) {
		fmt.Fprintf(os.Stdout, "Error: the argument to setq must be prime!\n")
		return
	}

	cf := new(big.Int)
	cf.GCD(nil, nil, q, &(CurKey.p))
	if cf.Cmp(one) != 0 {
		fmt.Fprintf(os.Stdout, "Error: p and q must be co-prime!\n")
		return
	}

	CurKey.q.Set(q)

	CurKey.n.Mul(&(CurKey.p), &(CurKey.q))
}


func set_e(t []string) {

	negone := big.NewInt(-1)
	one := big.NewInt(1)

	if (len(t) != 2) {
		fmt.Fprintf(os.Stdout, "Error: sete must have exactly one argument!\n")
		return
	}

	if CurKey.p.Cmp(negone) == 0 {
		fmt.Fprintf(os.Stdout, "Error: p (and q) must be set before e!\n")
		return
	}

	if CurKey.q.Cmp(negone) == 0 {
		fmt.Fprintf(os.Stdout, "Error: q must be set before e\n")
		return
	}

	if CurKey.e.Cmp(negone) != 0 {
		fmt.Fprintf(os.Stdout, "Unsetting previous e, d\n")
		CurKey.e.Set(negone)
		CurKey.d.Set(negone)
	}

	e := new(big.Int)
	_, ok := e.SetString(t[1], 10)

	if !ok {
		fmt.Fprintf(os.Stdout, "Error: could not parse numerical argument to sete!\n")
		return
	}

	pm1 := new(big.Int)
	qm1 := new(big.Int)
	pm1.Add(&(CurKey.p), negone)
	qm1.Add(&(CurKey.q), negone)

	cf := new(big.Int)
	cf.GCD(nil, nil, e, pm1)
	if cf.Cmp(one) != 0 {
		fmt.Fprintf(os.Stdout, "Error: e must be co-prime to p - 1!\n")
		return
	}

	cf.GCD(nil, nil, e, qm1)
	if cf.Cmp(one) != 0 {
		fmt.Fprintf(os.Stdout, "Error: e must be co-prime to q - 1!\n")
		return
	}

	etot := new(big.Int)
	etot.Mul(pm1, qm1)

	if e.Cmp(etot) >= 0 {
		fmt.Fprintf(os.Stdout, "Error: e must be less than (p - 1) * (q - 1)!\n")
		return
	}

	d := new(big.Int)
	d.ModInverse(e, etot)

	if d != nil {
		CurKey.e.Set(e)
		CurKey.d.Set(d)
	} else {
		fmt.Fprintf(os.Stdout, "Error: Unable to invert e to find d!\n")
		return
	}
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
