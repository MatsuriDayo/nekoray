package main

import (
	"fmt"
	"os"
	_ "unsafe"

	"github.com/v2fly/v2ray-core/v5/main/commands"
	"github.com/v2fly/v2ray-core/v5/main/commands/base"
)

//go:linkname build github.com/v2fly/v2ray-core/v5.build
var build string

var version_v2ray string = "N/A"
var version_standalone string = "N/A"

func main() {
	fmt.Println("V2Ray:", version_v2ray, "Version:", version_standalone)
	fmt.Println()

	// nekoray
	if len(os.Args) > 1 && os.Args[1] == "nekoray" {
		Main()
		return
	}

	// toolbox
	if len(os.Args) > 1 && os.Args[1] == "tool" {
		ToolBox(os.Args[2:])
		return
	}

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
