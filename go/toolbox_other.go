//go:build !windows && !linux

package main

import "flag"

func ToolBox(tool []string) {
	flag.Parse()
}
