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
			routes, err := NewRouteTable().getRoutes()
			if err != nil {
				log.Fatalln(err)
			}
			for _, route := range routes {
				log.Println(route)
			}
			log.Println("Upstream:", getBindInterfaceIndex())
		}
	}
}
