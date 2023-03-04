package main

import (
	"fmt"
	"grpc_server"
	"os"
	_ "unsafe"

	"github.com/matsuridayo/libneko/neko_common"

	"github.com/v2fly/v2ray-core/v5/main/commands"
	"github.com/v2fly/v2ray-core/v5/main/commands/base"
)

//go:linkname build github.com/v2fly/v2ray-core/v5.build
var build string

func main() {
	fmt.Println("V2Ray:", neko_common.Version_v2ray, "Neko:", neko_common.Version_neko)
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

	// v2ray
	build = "Matsuridayo/Nekoray"
	main_v2ray_v5()
}

func main_v2ray_v5() {
	base.RootCommand.Long = "A unified platform for anti-censorship."
	base.RegisterCommand(commands.CmdRun)
	base.RegisterCommand(commands.CmdVersion)
	base.RegisterCommand(commands.CmdTest)
	base.SortLessFunc = runIsTheFirst
	base.SortCommands()
	base.Execute()
}

func runIsTheFirst(i, j *base.Command) bool {
	left := i.Name()
	right := j.Name()
	if left == "run" {
		return true
	}
	if right == "run" {
		return false
	}
	return left < right
}
