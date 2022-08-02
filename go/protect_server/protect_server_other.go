//go:build !linux

package protect_server

import "log"

func ServeProtect(path string, fwmark int) {
	log.Println("ServeProtect is not for this platform")
}
