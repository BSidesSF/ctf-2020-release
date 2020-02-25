package main

import (
	"bytes"
	"encoding/base64"
	"fmt"
	"io/ioutil"
	"log"
	"math/rand"
	"os"
	"path/filepath"
	"strings"

	"github.com/google/uuid"
	"golang.org/x/crypto/bcrypt"
	"upper.io/db.v3"
	"upper.io/db.v3/lib/sqlbuilder"
	"upper.io/db.v3/mysql"
)

const (
	DBConnVar  = "MYSQL_DB"
	tblUsers   = "users"
	tblRecipes = "recipes"
	defaultDB  = "recipes:foobarbaz@tcp(localhost:3306)/recipes"
)

var dbConn sqlbuilder.Database

type User struct {
	ID       string `db:"id"`
	Username string `db:"username"`
	PWHash   string `db:"pwhash"`
}

func NewUser(username, pwd string) *User {
	ret := &User{
		Username: username,
		ID:       uuid.New().String(),
	}
	ret.SetPassword(pwd)
	return ret
}

func (u *User) SetPassword(pwd string) {
	g, err := bcrypt.GenerateFromPassword([]byte(pwd), bcrypt.MinCost)
	if err == nil {
		u.PWHash = string(g)
	}
}

func (u *User) VerifyPassword(pwd string) error {
	return bcrypt.CompareHashAndPassword([]byte(u.PWHash), []byte(pwd))
}

func (u *User) Insert() error {
	c := getDBConn().Collection(tblUsers)
	_, err := c.Insert(u)
	return err
}

func (u *User) CountRecipes() uint64 {
	c := getDBConn().Collection(tblRecipes)
	res := c.Find(db.Cond{"owner": u.ID})
	cnt, err := res.Count()
	if err != nil {
		log.Printf("Error loading recipes (count): %s", err)
		return 0
	}
	return cnt
}

func (u *User) GetRecipes() []*Recipe {
	c := getDBConn().Collection(tblRecipes)
	res := c.Find(db.Cond{"owner": u.ID})
	cnt, err := res.Count()
	if err != nil {
		log.Printf("Error loading recipes (count): %s", err)
		return nil
	}
	ret := make([]*Recipe, 0, cnt)
	if err := res.All(&ret); err != nil {
		log.Printf("Error loading recipes: %s", err)
		return nil
	}
	return ret
}

func (u *User) NewRecipe(name, body string) *Recipe {
	return &Recipe{
		Owner: u.ID,
		Name:  name,
		Body:  body,
	}
}

func GetByUsername(username string) *User {
	c := getDBConn().Collection(tblUsers)
	res := c.Find(db.Cond{"username": username})
	var u User
	if err := res.One(&u); err != nil {
		return nil
	}
	return &u
}

func GetByID(id string) *User {
	c := getDBConn().Collection(tblUsers)
	res := c.Find(db.Cond{"id": id})
	var u User
	if err := res.One(&u); err != nil {
		return nil
	}
	return &u
}

func GetUserList() []*User {
	c := getDBConn().Collection(tblUsers)
	res := c.Find()
	cnt, err := res.Count()
	if err != nil {
		log.Printf("Error loading users (count): %s", err)
		return nil
	}
	ret := make([]*User, 0, cnt)
	if err := res.All(&ret); err != nil {
		log.Printf("Error loading users: %s", err)
		return nil
	}
	return ret
}

type Recipe struct {
	ID    int    `db:"id"`
	Owner string `db:"owner"`
	Name  string `db:"name"`
	Body  string `db:"body"`
	Image string `db:"image"`
}

func (r *Recipe) Insert() error {
	c := getDBConn().Collection(tblRecipes)
	err := c.InsertReturning(r)
	return err
}

func GetRecipeById(id int) *Recipe {
	c := getDBConn().Collection(tblRecipes)
	res := c.Find(db.Cond{"id": id})
	var r Recipe
	if err := res.One(&r); err != nil {
		return nil
	}
	return &r
}

func getDBConn() sqlbuilder.Database {
	if dbConn != nil {
		return dbConn
	}
	dsn := os.Getenv(DBConnVar)
	if dsn == "" {
		dsn = defaultDB
	}
	settings, err := mysql.ParseURL(dsn)
	if err != nil {
		panic(err)
	}
	c, err := mysql.Open(settings)
	if err != nil {
		panic(err)
	}
	setupDB(c)
	dbConn = c
	return dbConn
}

func setupDB(c sqlbuilder.Database) {
	// Handle users
	u := c.Collection(tblUsers)
	r := c.Collection(tblRecipes)
	if u.Exists() && r.Exists() {
		return
	}
	if u.Exists() || r.Exists() {
		log.Println("Incomplete database setup!  Things may be broken!")
		return
	}
	c.Exec(`CREATE TABLE users (` +
		`id CHAR(100) PRIMARY KEY, ` +
		`username VARCHAR(100) UNIQUE NOT NULL, ` +
		`pwhash CHAR(100));`)
	c.Exec(`CREATE TABLE recipes (` +
		`id INTEGER NOT NULL PRIMARY KEY AUTO_INCREMENT, ` +
		`owner CHAR(100) NOT NULL, ` +
		`name VARCHAR(100) NOT NULL, ` +
		`body TEXT NOT NULL, ` +
		`image MEDIUMTEXT NOT NULL, ` +
		`INDEX owner_idx (owner));`)
	boudin := NewUser("boudin_bakery", "8c5edbb08f80890b82d585bbe38902d2")
	boudin.Insert()
	for _, r := range getRecipes("data/recipes/boudin") {
		r.Owner = boudin.ID
		r.Insert()
	}
	// Insert other users
	recipes := getRecipes("data/recipes/other")
	per_user := len(recipes) / 4
	for _, n := range []string{"mary_jane", "matir", "techbro"} {
		u := NewUser(n, "bf873b145ae3831c5b26e10a52686d77")
		u.Insert()
		perm := rand.Perm(len(recipes))
		for _, n := range perm[:per_user] {
			// Need to copy here
			var r Recipe
			r = *recipes[n]
			r.Owner = u.ID
			(&r).Insert()
		}
	}
}

func getRecipes(rpath string) []*Recipe {
	paths, _ := filepath.Glob(rpath + "/*")
	res := make([]*Recipe, 0, len(paths))
	for _, p := range paths {
		rpath := filepath.Join(p, "recipe.txt")
		var imgBlob string
		for _, imgName := range []string{"recipe.png", "recipe.jpg"} {
			ipath := filepath.Join(p, imgName)
			imgBlob = makeImageFileBlob(ipath)
			if imgBlob != "" {
				break
			}
		}
		if buf, err := ioutil.ReadFile(rpath); err != nil {
			log.Printf("Error reading %s: %s\n", p, err)
			continue
		} else {
			sbuf := string(buf)
			pieces := strings.SplitN(sbuf, "\n", 2)
			res = append(res, &Recipe{
				Name:  pieces[0],
				Body:  pieces[1],
				Image: imgBlob,
			})
		}
	}
	return res
}

func makeImageFileBlob(path string) string {
	buf, err := ioutil.ReadFile(path)
	if err != nil {
		return ""
	}
	return makeImageBlob(buf)
}

type magicEntry struct {
	magic       []byte
	contentType string
}

var magicMap = []magicEntry{
	magicEntry{
		[]byte{0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a},
		"image/png",
	},
	magicEntry{
		[]byte{0xFF, 0xD8, 0xFF},
		"image/jpeg",
	},
}

func makeImageBlob(buf []byte) string {
	content_type := "application/octet-stream"
	for _, e := range magicMap {
		if bytes.HasPrefix(buf, e.magic) {
			content_type = e.contentType
			break
		}
	}
	b64data := base64.StdEncoding.EncodeToString(buf)
	return fmt.Sprintf("data:%s;base64,%s", content_type, b64data)
}
