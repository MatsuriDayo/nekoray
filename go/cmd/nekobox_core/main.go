package main

import (
	"fmt"
	"os"
	_ "unsafe"

	"neko/pkg/grpc_server"
	"neko/pkg/neko_common"
	"nekobox_core/box_main"

	"github.com/sagernet/sing-box/constant"
)

func main() {
	fmt.Println("sing-box:", constant.Version, "Neko:", neko_common.Version_neko)
	fmt.Println()

	// nekobox_core
	if len(os.Args) > 1 && os.Args[1] == "nekobox" {
		neko_common.RunMode = neko_common.RunMode_NekoBox_Core
		grpc_server.RunCore(setupCore, &server{})
		return
	}

	// sing-box
	box_main.Main()
}
