package main

import (
	"html/template"
	"io"
	"log"
	"path/filepath"
)

const (
	baseTemplate = "templates/base.html"
	templateGlob = "templates/*"
)

var templateMap map[string]*template.Template

func executeTemplate(w io.Writer, name string, data interface{}) {
	if tmpl, ok := templateMap[name]; !ok {
		log.Printf("No template named %s", name)
	} else {
		if err := tmpl.ExecuteTemplate(w, name, data); err != nil {
			log.Printf("Error executing template: %s", err)
		}
	}
}

func init() {
	templateMap = make(map[string]*template.Template)
	paths, err := filepath.Glob(templateGlob)
	if err != nil {
		panic(err)
	}
	for _, f := range paths {
		if f == baseTemplate {
			continue
		}
		basename := filepath.Base(f)
		log.Printf("Parsing template %s", basename)
		templateMap[basename] = template.Must(template.ParseFiles(baseTemplate, f))
	}
}
