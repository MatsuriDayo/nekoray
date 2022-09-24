//go:build debug && !linux

package box_main

func rusageMaxRSS() float64 {
	return -1
}
