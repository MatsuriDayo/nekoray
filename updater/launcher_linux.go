package main

import (
	"flag"
	"log"
	"os"
	"os/exec"
	"path/filepath"
	"runtime"
)

var local_qt_theme bool

func Launcher() {
	log.Println("Running as launcher")
	wd, _ := filepath.Abs(".")

	_debug := flag.Bool("debug", false, "Debug mode")
	flag.BoolVar(&local_qt_theme, "theme", false, "Use local QT theme (unstable)")
	flag.Parse()

	// Find & symlink some Qt Plugin to enable system theme
	tryLinkQtPlugin("styles", !local_qt_theme)
	tryLinkQtPlugin("platformthemes", !local_qt_theme)

	cmd := exec.Command("./nekoray", flag.Args()...)
	cmd.Env = os.Environ()
	ld_env := "LD_LIBRARY_PATH=" + filepath.Join(wd, "./usr/lib")
	cmd.Env = append(cmd.Env, ld_env)
	log.Println(ld_env, cmd)

	if *_debug {
		cmd.Env = append(cmd.Env, "QT_DEBUG_PLUGINS=1")
		cmd.Stdin = os.Stdin
		cmd.Stderr = os.Stderr
		cmd.Stdout = os.Stdout
		cmd.Run()
	} else {
		cmd.Start()
	}
}

func tryLinkQtPlugin(sub string, remove bool) {
	wd_plugins_sub := filepath.Join("./usr/plugins", sub)

	if Exist(wd_plugins_sub) {
		if remove {
			os.RemoveAll(wd_plugins_sub)
		}
	} else {
		if remove {
			return
		}

		arch := "x86_64"
		if runtime.GOARCH == "arm64" {
			arch = "aarch64"
		}

		paths := []string{
			filepath.Join("/usr/lib/qt5/plugins", sub),
			filepath.Join("/usr/lib64/qt5/plugins", sub),
			filepath.Join("/usr/lib/"+arch+"-linux-gnu/qt5/plugins", sub),
			filepath.Join("/usr/lib/qt/plugins", sub),
		}
		path := FindExist(paths)
		if path == "" {
			log.Println("warning:", sub, "not found")
			return
		}

		err := os.Symlink(path, wd_plugins_sub)
		if err != nil {
			log.Println("symlink failed:", err.Error())
		}
	}
}
