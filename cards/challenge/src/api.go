package main

import (
	"encoding/json"
	"io/ioutil"
	"log"
	"net/http"
	"os"
	"strings"
)

func apiHandler(w http.ResponseWriter, r *http.Request) {
	if r.Method != "POST" {
		httpError(w, "Method not supported.", http.StatusMethodNotAllowed)
		return
	}
	path := strings.Split(r.URL.Path, "/")
	if len(path) < 3 {
		if err := json.NewEncoder(w).Encode(NewSession()); err != nil {
			httpError(w, "Internal Server Error", http.StatusInternalServerError)
		}
		return
	}
	body, err := ioutil.ReadAll(r.Body)
	if err != nil {
		httpError(w, "Internal Server Error", http.StatusInternalServerError)
		return
	}
	var session Session
	if err := json.Unmarshal(body, &session); err != nil {
		log.Printf("Error unmarshaling session: %s", err)
		httpError(w, "Bad Request", http.StatusBadRequest)
		return
	}
	switch path[2] {
	case "deal":
		if err := session.Deal(); err != nil {
			log.Printf("Error dealing: %s", err)
			httpError(w, err, http.StatusBadRequest)
			return
		}
	case "hit":
		if err := session.Hit(); err != nil {
			log.Printf("Error hitting: %s", err)
			httpError(w, err, http.StatusBadRequest)
			return
		}
	case "stand":
		if err := session.Stand(); err != nil {
			log.Printf("Error standing: %s", err)
			httpError(w, err, http.StatusBadRequest)
			return
		}
	case "double":
		if err := session.Double(); err != nil {
			log.Printf("Error doubling: %s", err)
			httpError(w, err, http.StatusBadRequest)
			return
		}
	default:
		httpError(w, "Not Found", http.StatusNotFound)
		return
	}
	if err := json.NewEncoder(w).Encode(&session); err != nil {
		httpError(w, "Internal Server Error", http.StatusInternalServerError)
	}
}

func configHandler(w http.ResponseWriter, r *http.Request) {
	cfg := struct {
		Goal        int
		MinBet      int
		MaxBet      int
		GameHandler string
		DeckHandler string
	}{
		Goal:        AmountPlayerWins,
		MinBet:      MinBet,
		MaxBet:      MaxBet,
		GameHandler: "/game.go",
		DeckHandler: "/deck.go",
	}
	if err := json.NewEncoder(w).Encode(cfg); err != nil {
		httpError(w, "Internal Server Error", http.StatusInternalServerError)
	}
}

func httpError(w http.ResponseWriter, errMsg interface{}, code int) {
	var errString string
	var jsonObj struct {
		Error string `json:"error"`
	}
	switch v := errMsg.(type) {
	case string:
		jsonObj.Error = v
		if buf, err := json.Marshal(jsonObj); err != nil {
			errString = "Unknown Error"
		} else {
			errString = string(buf)
		}
	case error:
		jsonObj.Error = v.Error()
		if buf, err := json.Marshal(jsonObj); err != nil {
			errString = "Unknown Error"
		} else {
			errString = string(buf)
		}
	default:
		if buf, err := json.Marshal(v); err != nil {
			errString = "Unknown Error"
		} else {
			errString = string(buf)
		}
	}
	http.Error(w, errString, code)
}

func init() {
	log.SetOutput(os.Stderr)
	http.HandleFunc("/api/config", configHandler)
	http.HandleFunc("/api/", apiHandler)
	http.HandleFunc("/api", apiHandler)
	http.Handle("/", http.FileServer(http.Dir("static")))
}

func main() {
	log.Fatal(http.ListenAndServe(":8080", nil))
}
