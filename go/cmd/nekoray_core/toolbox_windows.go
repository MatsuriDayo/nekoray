package main

import (
	"log"
	"net"
	"os"
)

func ToolBox() {
	switch os.Args[2] {
	case "if":
		{
			intfs, err := net.Interfaces()
			if err != nil {
				log.Fatalln(err)
			}
			for _, intf := range intfs {
				log.Println(intf)
			}
			for _, route := range routes {
				log.Println(route)
			}
			log.Println("TUN:", getNekorayTunIndex())
			log.Println("Upstream:", getBindInterfaceIndex("8.8.8.8:8888"))
		}
	}
}
