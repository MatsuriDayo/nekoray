package main

import (
	"net"

	"github.com/v2fly/v2ray-core/v5/nekoutils"
)

func init() {
	setupCore_platforms = append(setupCore_platforms, func() {
		nekoutils.Windows_Protect_BindInterfaceIndex = windows_Protect_BindInterfaceIndex
	})
}

func windows_Protect_BindInterfaceIndex() uint32 {
	intfs, err := net.Interfaces()
	if err != nil {
		return 0
	}
	// TODO hardcoded tun name "wintun"
	if len(intfs) > 1 {
		if intfs[0].Name == "wintun" || intfs[0].Name == "TunMax" {
			return uint32(intfs[1].Index)
		}
	}
	return 0
}
