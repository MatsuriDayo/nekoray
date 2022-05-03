package main

import (
	"log"
	"os"
	"os/exec"
	"path/filepath"
)

func Launcher() {
	log.Println("Running as launcher")

	wd, _ := filepath.Abs(".")

	// Find & symlink some Qt Plugin to enable system theme
	tryLink("styles")
	tryLink("platformthemes")

	// updater
	exec.Command("sh", "-c", "ln -sf launcher updater").Run()

	cmd := exec.Command("./nekoray")
	cmd.Env = os.Environ()
	cmd.Env = append(cmd.Env, "LD_LIBRARY_PATH="+filepath.Join(wd, "./usr/lib"))
	cmd.Start()
}

func tryLink(sub string) {
	wd_plugins := filepath.Join("./usr/plugins", sub)

	if !Exist(wd_plugins) {
		paths := []string{
			filepath.Join("/usr/lib/qt/plugins", sub),
			filepath.Join("/usr/lib/qt5/plugins", sub),
			filepath.Join("/usr/lib/qt5/plugins", sub),
			filepath.Join("/usr/lib/x86_64-linux-gnu/qt5/plugins", sub),
		}
		path := FindExist(paths)
		if path == "" {
			log.Println(sub, "not found")
			return
		}

		err := os.Symlink(path, wd_plugins)
		if err != nil {
			log.Fatalln("symlink failed", err.Error())
		}
	}

}
