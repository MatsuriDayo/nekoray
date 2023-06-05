package main

import (
	"context"
	"log"
	"os"
	"path/filepath"
	"runtime"
	"strings"

	"github.com/codeclysm/extract"
)

func Updater() {
	pre_cleanup := func() {
		if runtime.GOOS == "linux" {
			os.RemoveAll("./usr")
		}
		os.RemoveAll("./nekoray_update")
	}

	// find update package
	var updatePackagePath string
	if len(os.Args) == 2 && Exist(os.Args[1]) {
		updatePackagePath = os.Args[1]
	} else if Exist("./nekoray.zip") {
		updatePackagePath = "./nekoray.zip"
	} else if Exist("./nekoray.tar.gz") {
		updatePackagePath = "./nekoray.tar.gz"
	} else {
		log.Fatalln("no update")
	}
	log.Println("updating from", updatePackagePath)

	// extract update package
	if strings.HasSuffix(updatePackagePath, ".zip") {
		pre_cleanup()
		f, err := os.Open(updatePackagePath)
		if err != nil {
			log.Fatalln(err.Error())
		}
		err = extract.Zip(context.Background(), f, "./nekoray_update", nil)
		if err != nil {
			log.Fatalln(err.Error())
		}
		f.Close()
	} else if strings.HasSuffix(updatePackagePath, ".tar.gz") {
		pre_cleanup()
		f, err := os.Open(updatePackagePath)
		if err != nil {
			log.Fatalln(err.Error())
		}
		err = extract.Gz(context.Background(), f, "./nekoray_update", nil)
		if err != nil {
			log.Fatalln(err.Error())
		}
		f.Close()
	}

	// remove old file
	removeAll("./*.dll")
	removeAll("./*.dmp")

	// update move
	err := Mv("./nekoray_update/nekoray", "./")
	if err != nil {
		MessageBoxPlain("NekoGui Updater", "Update failed. Please close the running instance and run the updater again.\n\n"+err.Error())
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
