package main

import (
	"fmt"
	"os"
	_ "unsafe"

	"grpc_server"

	"github.com/matsuridayo/libneko/neko_common"
	"github.com/matsuridayo/sing-box-extra/boxmain"
	_ "github.com/matsuridayo/sing-box-extra/distro/all"

	"github.com/sagernet/sing-box/constant"
)

func main() {
	fmt.Println("sing-box:", constant.Version, "NekoBox:", neko_common.Version_neko)
	fmt.Println()

	// local DNS transport
	_ = os.Setenv("GODEBUG", os.Getenv("GODEBUG")+",netdns=go")

	// nekobox_core
	if len(os.Args) > 1 && os.Args[1] == "nekobox" {
		neko_common.RunMode = neko_common.RunMode_NekoBox_Core
		grpc_server.RunCore(setupCore, &server{})
		return
	}

	// sing-box
	boxmain.Main()
}
