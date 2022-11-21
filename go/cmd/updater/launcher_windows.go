package main

import (
	"log"
	"runtime"
)

func Launcher() {
	log.Fatalf("launcher is not for your platform", runtime.GOOS)
}
