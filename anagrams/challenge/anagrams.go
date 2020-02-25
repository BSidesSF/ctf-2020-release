package main

import (
	"bufio"
	"fmt"
	"os"
	"strings"
)

func main() {
	words := loadDictionary()
	if len(os.Args) < 2 {
		fmt.Println("Need words to scramble!")
		return
	}
	buf := strings.Join(os.Args[1:], "")
	for _, r := range findAnagrams(buf, words) {
		fmt.Println(r)
	}
}

func loadDictionary() []string {
	fp, err := os.Open("english.dic")
	if err != nil {
		panic(err)
	}
	defer fp.Close()
	words := make([]string, 0)
	scanner := bufio.NewScanner(fp)
	for scanner.Scan() {
		words = append(words, scanner.Text())
	}
	return words
}

func wordInString(word, buf string) (bool, string) {
	if len(word) > len(buf) {
		return false, buf
	}
	charset := []rune(buf)
	for _, c := range word {
		foundC := -1
		for i, r := range charset {
			if r == c {
				foundC = i
				break
			}
		}
		if foundC == -1 {
			return false, buf
		} else {
			charset = append(charset[:foundC], charset[foundC+1:]...)
		}
	}
	return true, string(charset)
}

func findAnagrams(buf string, dict []string) []string {
	var finder func(buf string, pre []string) [][]string
	finder = func(buf string, pre []string) [][]string {
		res := make([][]string, 0)
		for _, w := range dict {
			if contained, left := wordInString(w, buf); contained {
				if left != "" {
					res = append(res, finder(left, append(pre, w))...)
				} else {
					res = append(res, append(pre, w))
				}
			}
		}
		return res
	}
	results := make([]string, 0)
	for _, r := range finder(buf, nil) {
		results = append(results, strings.Join(r, " "))
	}
	return results
}
