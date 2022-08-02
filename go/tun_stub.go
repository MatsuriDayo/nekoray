//go:build !linux

package main

import (
	"errors"
	"libcore"
	"nekoray_core/gen"
)

func TunStart(config *gen.SetTunReq) error {
	return errors.New("not for this platform")
}

func TunStop() {
}

func TunSetV2ray(i *libcore.V2RayInstance) {
}
