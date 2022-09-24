//go:build !linux

package protect_server

func ServeProtect(path string, fwmark int) {}
