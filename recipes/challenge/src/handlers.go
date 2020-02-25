package main

import (
	"fmt"
	"html/template"
	"io/ioutil"
	"log"
	"net"
	"net/http"
	"net/url"
	"os"
	"regexp"
	"strconv"
	"strings"
	"time"

	"github.com/go-sql-driver/mysql"
	"recipes/jwt"
)

const (
	CookieName = "auth_token"
	Issuer     = "recipebot"
)

var (
	signingSecret   = []byte("foobarbazbang!")
	blockedUsername = regexp.MustCompile(".*boudin.*")
)

type AuthenticatedHandlerFunc func(http.ResponseWriter, *http.Request, *User)
type HandlerFuncWithUser func(http.ResponseWriter, *http.Request, *User)

func (f AuthenticatedHandlerFunc) ServeHTTP(w http.ResponseWriter, r *http.Request) {
	user := GetUserFromRequest(r)
	if user == nil {
		http.Error(w, "Not Authorized", http.StatusUnauthorized)
		return
	}
	// refresh token
	addCookieForUser(w, user)
	f(w, r, user)
}

func (f HandlerFuncWithUser) ServeHTTP(w http.ResponseWriter, r *http.Request) {
	user := GetUserFromRequest(r)
	if user != nil {
		// refresh token
		addCookieForUser(w, user)
	}
	f(w, r, user)
}

func GetUserFromRequest(r *http.Request) *User {
	if c, err := r.Cookie(CookieName); err != nil {
		log.Println("No auth_token.")
		return nil
	} else {
		token, err := jwt.ParseWithClaims(c.Value, &jwt.StandardClaims{}, getSigningSecret)
		if err != nil {
			log.Printf("Error parsing token: %s\n", err)
			return nil
		}
		if !token.Valid {
			log.Println("Invalid token.")
			return nil
		}
		if claims, ok := token.Claims.(*jwt.StandardClaims); ok {
			if !claims.VerifyIssuer(Issuer, true) {
				log.Println("Invalid Issuer")
				return nil
			}
			user := GetByID(claims.Subject)
			if user == nil {
				log.Println("Invalid User")
				return nil
			}
			log.Printf("Request for user %s to path %s", user.ID, r.URL.Path)
			return user
		} else {
			log.Println("Error Getting Claims!")
			return nil
		}
	}
}

func getSigningSecret(token *jwt.Token) (interface{}, error) {
	return signingSecret, nil
}

func getSignedJWTForUser(u *User) string {
	now := time.Now()
	claims := jwt.StandardClaims{
		Subject:   u.ID,
		Issuer:    Issuer,
		IssuedAt:  now.Unix(),
		NotBefore: now.Unix(),
		ExpiresAt: now.Add(time.Hour).Unix(),
	}
	token := jwt.NewWithClaims(jwt.SigningMethodHS256, claims)
	tokenString, err := token.SignedString(signingSecret)
	if err != nil {
		fmt.Fprintf(os.Stderr, "Error signing jwt: %s\n", err)
	}
	return tokenString
}

func addCookieForUser(w http.ResponseWriter, u *User) {
	if token := getSignedJWTForUser(u); token != "" {
		c := &http.Cookie{
			Name:     CookieName,
			Value:    token,
			HttpOnly: true,
			Expires:  time.Now().Add(time.Hour),
			Path:     "/",
		}
		http.SetCookie(w, c)
	}
}

func clearCookie(w http.ResponseWriter) {
	http.SetCookie(w, &http.Cookie{
		Name:     CookieName,
		Value:    "",
		Expires:  time.Now().Add(-time.Hour),
		MaxAge:   0,
		HttpOnly: true,
		Path:     "/",
	})
}

func isFromLocalhost(r *http.Request) bool {
	allowed := []string{"127.0.0.1", "::1"}
	remote, _, _ := net.SplitHostPort(r.RemoteAddr)
	lookup, _ := net.LookupHost(remote)
	for _, l := range lookup {
		for _, a := range allowed {
			if a == l {
				return true
			}
		}
	}
	return false
}

// Page handlers
func homePageHandler(w http.ResponseWriter, r *http.Request, u *User) {
	if r.URL.Path != "/" {
		http.NotFound(w, r)
		return
	}
	data := struct {
		User *User
	}{
		u,
	}
	executeTemplate(w, "index.html", data)
}

func recipeListHandler(w http.ResponseWriter, r *http.Request, u *User) {
	data := struct {
		User    *User
		Recipes []*Recipe
	}{
		u,
		u.GetRecipes(),
	}
	executeTemplate(w, "recipes.html", data)
}

func getRecipeHandler(w http.ResponseWriter, r *http.Request, u *User) {
	which := strings.TrimPrefix(r.URL.Path, "/recipe/")
	rid, err := strconv.Atoi(which)
	if err != nil {
		http.NotFound(w, r)
		return
	}
	rec := GetRecipeById(rid)
	if rec == nil {
		http.NotFound(w, r)
		return
	}
	if rec.Owner != u.ID {
		http.Error(w, "Not Authorized", http.StatusUnauthorized)
		return
	}
	data := struct {
		User           *User
		Recipe         *Recipe
		SanitizedImage template.URL
	}{
		u,
		rec,
		template.URL(rec.Image),
	}
	executeTemplate(w, "recipe.html", data)
}

func addRecipeHandler(w http.ResponseWriter, r *http.Request, u *User) {
	errRender := func(err string) {
		data := struct {
			User  *User
			Error string
		}{
			u,
			err,
		}
		executeTemplate(w, "add.html", data)
	}

	if r.Method == "GET" {
		errRender("")
		return
	} else if r.Method != "POST" {
		http.Error(w, "Not Allowed", http.StatusMethodNotAllowed)
		return
	}

	name := r.FormValue("name")
	body := r.FormValue("body")
	if name == "" {
		errRender("Name is required.")
		return
	}
	if body == "" {
		errRender("Recipe text is required.")
		return
	}
	recipe := u.NewRecipe(name, body)
	imageUrl := r.FormValue("image_url")
	if imageUrl != "" {
		if imgu, err := url.Parse(imageUrl); err != nil {
			errRender("Invalid image URL")
			return
		} else {
			if imgu.Scheme != "http" && imgu.Scheme != "https" {
				errRender("Only http/https are supported.")
				return
			}
			if imgBody, err := getImageBytes(imgu.String()); err != nil {
				log.Printf("Error retrieving image: %s", err)
				errRender("Error retrieving image!")
				return
			} else {
				recipe.Image = makeImageBlob(imgBody)
			}
		}
	}
	if err := recipe.Insert(); err != nil {
		log.Printf("Error inserting record: %s", err)
		http.Error(w, "Server Error", http.StatusInternalServerError)
		return
	}
	// Redirect to finished page
	recipeUrl := fmt.Sprintf("/recipe/%d", recipe.ID)
	http.Redirect(w, r, recipeUrl, http.StatusFound)
}

func getImageBytes(u string) ([]byte, error) {
	client := GetSafeHTTPClient()
	req, err := http.NewRequest(http.MethodGet, u, nil)
	if err != nil {
		return nil, err
	}
	req.Header.Set("User-Agent", "RecipeServ/1.0")
	resp, err := client.Do(req)
	if err != nil {
		return nil, err
	}
	defer resp.Body.Close()
	return ioutil.ReadAll(resp.Body)
}

func userListHandler(w http.ResponseWriter, r *http.Request) {
	if !isFromLocalhost(r) {
		w.WriteHeader(http.StatusUnauthorized)
		data := struct {
			User     *User
			Hostlink string
		}{
			nil,
			ListenAddress,
		}
		executeTemplate(w, "localhost.html", data)
		return
	}
	data := struct {
		User     *User
		UserList []*User
	}{
		UserList: GetUserList(),
	}
	executeTemplate(w, "users.html", data)
}

func registerHandler(w http.ResponseWriter, r *http.Request) {
	if r.Method == "GET" {
		executeTemplate(w, "register.html", nil)
		return
	} else if r.Method != "POST" {
		http.Error(w, "Not Allowed", http.StatusMethodNotAllowed)
		return
	}
	username := r.FormValue("username")
	password := r.FormValue("password")
	confirm := r.FormValue("password2")
	if blockedUsername.MatchString(username) {
		data := struct {
			User     *User
			Username string
			Error    string
		}{
			Username: username,
			Error:    "Username not allowed!",
		}
		executeTemplate(w, "register.html", data)
		return
	}
	if password != confirm {
		data := struct {
			User     *User
			Username string
			Error    string
		}{
			Username: username,
			Error:    "Passwords don't match.",
		}
		executeTemplate(w, "register.html", data)
		return
	}
	u := NewUser(username, password)
	err := u.Insert()
	if err != nil {
		emsg := "Unknown error"
		if mysqlErr, ok := err.(*mysql.MySQLError); ok {
			log.Printf("MySQL Error: %s", err)
			if mysqlErr.Number == 1062 {
				emsg = "Username already exists."
			}
		}
		data := struct {
			User     *User
			Username string
			Error    string
		}{
			Username: username,
			Error:    emsg,
		}
		executeTemplate(w, "register.html", data)
		return
	}
	addCookieForUser(w, u)
	http.Redirect(w, r, "/", http.StatusFound)
}

func loginHandler(w http.ResponseWriter, r *http.Request) {
	if r.Method == "GET" {
		executeTemplate(w, "login.html", nil)
		return
	} else if r.Method != "POST" {
		http.Error(w, "Not Allowed", http.StatusMethodNotAllowed)
		return
	}
	username := r.FormValue("username")
	password := r.FormValue("password")
	u := GetByUsername(username)
	if u == nil || u.VerifyPassword(password) != nil {
		data := struct {
			User     *User
			Username string
			Error    string
		}{
			Username: username,
			Error:    "Invalid username/password.",
		}
		executeTemplate(w, "login.html", data)
		return
	}
	addCookieForUser(w, u)
	http.Redirect(w, r, "/", http.StatusFound)
}

func logoutHandler(w http.ResponseWriter, r *http.Request) {
	clearCookie(w)
	http.Redirect(w, r, "/", http.StatusFound)
}

func init() {
	http.Handle("/", HandlerFuncWithUser(homePageHandler))
	http.Handle("/recipes", AuthenticatedHandlerFunc(recipeListHandler))
	http.Handle("/recipe/", AuthenticatedHandlerFunc(getRecipeHandler))
	http.Handle("/recipe/new", AuthenticatedHandlerFunc(addRecipeHandler))
	http.HandleFunc("/users", userListHandler)
	http.HandleFunc("/register", registerHandler)
	http.HandleFunc("/login", loginHandler)
	http.HandleFunc("/logout", logoutHandler)
	http.Handle("/static/", http.StripPrefix("/static", http.FileServer(http.Dir("./static"))))
}
