package main

import (
	"io/ioutil"
	"log"
	"os"
	"os/exec"
	"path/filepath"
	"runtime"
	"strings"
	"time"
)

func main() {
	// update & launcher
	exe, err := os.Executable()
	if err != nil {
		panic(err.Error())
	}

	wd := filepath.Dir(exe)
	os.Chdir(wd)
	exe = filepath.Base(os.Args[0])
	log.Println("exe:", exe, "exe dir:", wd)

	if strings.HasPrefix(strings.ToLower(exe), "updater") {
		if runtime.GOOS == "windows" {
			if strings.HasPrefix(strings.ToLower(exe), "updater.old") {
				// 2. "updater.old" update files
				time.Sleep(time.Second)
				Updater()
				// 3. start
				exec.Command("./nekoray.exe").Start()
			} else {
				// 1. nekoray stop it self and run "updater.exe"
				Copy("./updater.exe", "./updater.old")
				exec.Command("./updater.old", os.Args[1:]...).Start()
			}
		} else {
			// 1. update files
			Updater()
			// 2. start
			if os.Getenv("NKR_FROM_LAUNCHER") == "1" {
				Launcher()
			} else {
				exec.Command("./nekoray").Start()
			}
		}
		return
	} else if strings.HasPrefix(strings.ToLower(exe), "launcher") {
		Launcher()
		return
	}
	log.Fatalf("wrong name")
}

func Copy(src string, dst string) {
	// Read all content of src to data
	data, _ := ioutil.ReadFile(src)
	// Write data to dst
	ioutil.WriteFile(dst, data, 0644)
}
