//go:build !windows

package main

func MessageBoxPlain(title, caption string) int {
	return 0
}
