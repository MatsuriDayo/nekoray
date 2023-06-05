//go:build !linux

package main

import (
	"log"
	"runtime"
)

func Launcher() {
	log.Fatalln("launcher is not for your platform", runtime.GOOS)
}
