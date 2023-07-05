package main

import (
	"fmt"
	"grpc_server"
	"os"
	_ "unsafe"

	"github.com/matsuridayo/libneko/neko_common"
)

//go:linkname build github.com/xtls/xray-core/core.build
var build string

func main() {
	build = "Matsuridayo/nekoray"
	fmt.Println("Xray:", neko_common.Version_v2ray, "NekoRay:", neko_common.Version_neko)
	fmt.Println()

	// nekoray_core
	if len(os.Args) > 1 && os.Args[1] == "nekoray" {
		neko_common.RunMode = neko_common.RunMode_NekoRay_Core
		grpc_server.RunCore(setupCore, &server{})
		return
	}

	// tool
	if len(os.Args) > 1 && os.Args[1] == "tool" {
		ToolBox()
		return
	}
}
