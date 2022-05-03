package main

import (
	"encoding/json"
	"log"
	"os"
	"path/filepath"
	"runtime"
	"updater/unzip"
)

func Updater() {
	// delete
	if f, _ := os.Open("./del.json"); f != nil {
		v := &struct {
			Files []string
		}{}
		err := json.NewDecoder(f).Decode(v)
		if err == nil {
			for _, fn := range v.Files {
				log.Println("removeing", fn, os.RemoveAll(fn))
			}
		}
		os.Remove("./del.json")
	}

	// update
	if !Exist("./nekoray.zip") {
		log.Fatalln("no update")
	}

	log.Println("updating from zip")

	if runtime.GOOS == "linux" {
		os.RemoveAll("./usr")
	}

	os.RemoveAll("./nekoray_update")
	err := unzip.New("./nekoray.zip", "./nekoray_update").Extract()
	if err != nil {
		log.Fatalln(err.Error())
	}

	err = Mv("./nekoray_update/nekoray", "./")
	if err != nil {
		log.Fatalln(err.Error())
	}

	os.RemoveAll("./nekoray_update")
	os.RemoveAll("./nekoray.zip")
}

func Exist(path string) bool {
	_, err := os.Stat(path)
	return err == nil
}

func FindExist(paths []string) string {
	for _, path := range paths {
		if Exist(path) {
			return path
		}
	}
	return ""
}

func Mv(src, dst string) error {
	s, err := os.Stat(src)
	if err != nil {
		return err
	}
	if s.IsDir() {
		es, err := os.ReadDir(src)
		if err != nil {
			return err
		}
		for _, e := range es {
			err = Mv(filepath.Join(src, e.Name()), filepath.Join(dst, e.Name()))
			if err != nil {
				return err
			}
		}
	} else {
		err = os.MkdirAll(filepath.Dir(dst), 0755)
		if err != nil {
			return err
		}
		err = os.Rename(src, dst)
		if err != nil {
			return err
		}
	}
	return nil
}
