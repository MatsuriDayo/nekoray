package main

import (
	"flag"
	"log"
	"os"
	"os/exec"
	"path/filepath"
	"runtime"
)

func Launcher() {
	log.Println("Running as launcher")

	_debug := flag.Bool("debug", false, "debug")
	flag.Parse()

	wd, _ := filepath.Abs(".")

	// Find & symlink some Qt Plugin to enable system theme
	tryLink("styles")
	tryLink("platformthemes")

	// updater
	exec.Command("sh", "-c", "ln -sf launcher updater").Run()

	cmd := exec.Command("./nekoray")
	cmd.Env = os.Environ()
	cmd.Env = append(cmd.Env, "LD_LIBRARY_PATH="+filepath.Join(wd, "./usr/lib"))

	if *_debug {
		cmd.Env = append(cmd.Env, "QT_DEBUG_PLUGINS=1")
		cmd.Run()
	} else {
		cmd.Start()
	}
}

func tryLink(sub string) {
	wd_plugins := filepath.Join("./usr/plugins", sub)

	arch := "x86_64"
	if runtime.GOARCH == "arm64" {
		arch = "aarch64"
	}

	if !Exist(wd_plugins) {
		paths := []string{
			filepath.Join("/usr/lib/qt5/plugins", sub),
			filepath.Join("/usr/lib64/qt5/plugins", sub),
			filepath.Join("/usr/lib/"+arch+"-linux-gnu/qt5/plugins", sub),
			filepath.Join("/usr/lib/qt/plugins", sub),
		}
		path := FindExist(paths)
		if path == "" {
			log.Println(sub, "not found")
			return
		}

		err := os.Symlink(path, wd_plugins)
		if err != nil {
			log.Println("symlink failed:", err.Error())
		}
	}

}
