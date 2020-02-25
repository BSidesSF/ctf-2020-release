package main

import (
	"fmt"
	"net/http"
	"regexp"
	"strconv"
	"strings"
)

const FLAG = "CTF{I_have_been_expecting_U}"

var hurdles []func(*http.Request) error

func Home(w http.ResponseWriter, r *http.Request) {
	fmt.Fprintf(w, "You'll be rewarded with a flag if you can make it over some /hurdles.")
}

func Hurdles(w http.ResponseWriter, r *http.Request) {
	for i, h := range hurdles {
		if e := h(r); e != nil {
			w.Header().Add("X-Hurdles-Remaining", fmt.Sprintf("%d", len(hurdles)-i))
			w.WriteHeader(http.StatusUnauthorized)
			fmt.Fprintf(w, "I'm sorry, %s\n", e)
			return
		}
	}
	w.Header().Add("X-CTF-Flag", FLAG)
	fmt.Fprintf(w, "Congratulations!")
}

func addHurdle(f ...func(*http.Request) error) {
	hurdles = append(hurdles, f...)
}

func addAllHurdles() {
	addHurdle(
		func(r *http.Request) error {
			if r.Method != http.MethodPut {
				return fmt.Errorf("I was expecting the PUT Method.")
			}
			return nil
		},
		func(r *http.Request) error {
			if !strings.HasSuffix(r.URL.Path, "/!") {
				return fmt.Errorf("Your path would be more exciting if it ended in !")
			}
			return nil
		},
		func(r *http.Request) error {
			v := r.URL.Query()
			if v.Get("get") != "flag" {
				return fmt.Errorf("Your URL did not ask to `get` the `flag` in its query string.")
			}
			return nil
		},
		func(r *http.Request) error {
			k := "&=&=&"
			v := "%00\n"
			if r.FormValue(k) == "" {
				return fmt.Errorf("I was looking for a parameter named %s", k)
			}
			if r.FormValue(k) != v {
				return fmt.Errorf("I expected '%s' to equal '%s'", k, v)
			}
			return nil
		},
		func(r *http.Request) error {
			u, p, ok := r.BasicAuth()
			if !ok || u != "player" {
				return fmt.Errorf("Basically, I was expecting the username player.")
			}
			if p != "54ef36ec71201fdf9d1423fd26f97f6b" {
				return fmt.Errorf("Basically, I was expecting the password of the hex representation of the md5 of the string 'open sesame'")
			}
			return nil
		},
		func(r *http.Request) error {
			u := r.UserAgent()
			if !strings.Contains(u, "1337") {
				return fmt.Errorf("I was expecting you to be using a 1337 Browser.")
			}
			e9k := fmt.Errorf("I was expecting your browser version (v.XXXX) to be over 9000!")
			if !strings.Contains(u, "v.") {
				return e9k
			}
			re := regexp.MustCompile(`v\.(\d+)`)
			res := re.FindStringSubmatch(u)
			if len(res) != 2 {
				return e9k
			}
			ver, err := strconv.Atoi(res[1])
			if err != nil || ver < 9000 {
				return e9k
			}
			return nil
		},
		func(r *http.Request) error {
			hosts := r.Header.Get("X-Forwarded-For")
			if hosts == "" {
				return fmt.Errorf("I was eXpecting this to be Forwarded-For someone!")
			}
			s := stripStrings(strings.Split(hosts, ","))
			if len(s) < 2 {
				return fmt.Errorf("I was eXpecting this to be Forwarded For someone through another proxy!")
			}
			cli_ip := "13.37.13.37"
			proxy_ip := "127.0.0.1"
			if s[1] != proxy_ip {
				return fmt.Errorf("I was expecting this to be forwarded through %s", proxy_ip)
			}
			if s[0] != cli_ip {
				return fmt.Errorf("I was expecting the forwarding client to be %s", cli_ip)
			}
			return nil
		},
		func(r *http.Request) error {
			ckName := "Fortune"
			c, err := r.Cookie(ckName)
			if err != nil {
				return fmt.Errorf("I was expecting a %s Cookie", ckName)
			}
			ckVal := "6265"
			if !strings.Contains(c.Value, ckVal) {
				return fmt.Errorf("I was expecting the cookie to contain the number of the HTTP Cookie (State Management Mechanism) RFC from 2011.")
			}
			return nil
		},
		func(r *http.Request) error {
			if !strings.HasPrefix(r.Header.Get("Accept"), "text/plain") {
				return fmt.Errorf("I expect you to accept only plain text media (MIME) type.")
			}
			return nil
		},
		func(r *http.Request) error {
			lang := "ru"
			if !strings.HasPrefix(r.Header.Get("Accept-Language"), lang) {
				return fmt.Errorf("Я ожидал, что вы говорите по-русски.")
			}
			return nil
		},
		func(r *http.Request) error {
			expOrigin := "https://ctf.bsidessf.net"
			if r.Header.Get("Origin") != expOrigin {
				return fmt.Errorf("I was expecting to share resources with the origin %s", expOrigin)
			}
			return nil
		},
		func(r *http.Request) error {
			refHdr := "https://ctf.bsidessf.net/challenges"
			if r.Header.Get("Referer") != refHdr {
				return fmt.Errorf("I was expecting you would be refered by %s?", refHdr)
			}
			return nil
		},
	)
}

func stripStrings(v []string) []string {
	rv := make([]string, 0, len(v))
	for _, s := range v {
		rv = append(rv, strings.TrimSpace(s))
	}
	return rv
}

func main() {
	addAllHurdles()
	http.HandleFunc("/", Home)
	http.HandleFunc("/hurdles", Hurdles)
	http.HandleFunc("/hurdles/", Hurdles)
	http.ListenAndServe(":8888", nil)
}
