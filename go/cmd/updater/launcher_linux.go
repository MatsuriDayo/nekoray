package main

import (
	"flag"
	"log"
	"os"
	"os/exec"
	"path/filepath"
)

var local_qt_theme bool

func Launcher() {
	log.Println("Running as launcher")
	wd, _ := filepath.Abs(".")

	_debug := flag.Bool("debug", false, "Debug mode")
	flag.Parse()

	cmd := exec.Command("./nekoray", flag.Args()...)

	ld_env := "LD_LIBRARY_PATH=" + filepath.Join(wd, "./usr/lib")
	qt_plugin_env := "QT_PLUGIN_PATH=" + filepath.Join(wd, "./usr/plugins")

	// Qt 5.12 abi is usually compatible with system Qt 5.15
	// But use package Qt 5.12 by default.
	cmd.Env = os.Environ()
	cmd.Env = append(cmd.Env, "NKR_FROM_LAUNCHER=1")
	cmd.Env = append(cmd.Env, ld_env, qt_plugin_env)
	log.Println(ld_env, qt_plugin_env, cmd)

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
