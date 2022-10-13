package main

import (
	"context"
	"log"
	"os"
	"path/filepath"
	"runtime"

	"github.com/codeclysm/extract"
)

func Updater() {
	pre_cleanup := func() {
		if runtime.GOOS == "linux" {
			os.RemoveAll("./usr")
		}
		os.RemoveAll("./nekoray_update")
	}

	// update extract
	if Exist("./nekoray.zip") {
		pre_cleanup()
		log.Println("updating from zip")

		f, err := os.Open("./nekoray.zip")
		if err != nil {
			log.Fatalln(err.Error())
		}
		err = extract.Zip(context.Background(), f, "./nekoray_update", nil)
		if err != nil {
			log.Fatalln(err.Error())
		}
		f.Close()
	} else if Exist("./nekoray.tar.gz") {
		pre_cleanup()
		log.Println("updating from tar.gz")

		f, err := os.Open("./nekoray.tar.gz")
		if err != nil {
			log.Fatalln(err.Error())
		}
		err = extract.Gz(context.Background(), f, "./nekoray_update", nil)
		if err != nil {
			log.Fatalln(err.Error())
		}
		f.Close()
	} else {
		log.Fatalln("no update")
	}

	// remove old file
	removeAll("./*.dll")
	removeAll("./*.dmp")

	// update move
	err := Mv("./nekoray_update/nekoray", "./")
	if err != nil {
		log.Fatalln(err.Error())
	}

	os.RemoveAll("./nekoray_update")
	os.RemoveAll("./nekoray.zip")
	os.RemoveAll("./nekoray.tar.gz")
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

func removeAll(glob string) {
	files, _ := filepath.Glob(glob)
	for _, f := range files {
		os.Remove(f)
	}
}
