package main

import (
	"log"
	"net"
)

func ToolBox(tool []string) {
	switch tool[0] {
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
			log.Println("Upstream:", windows_Protect_BindInterfaceIndex())
		}
	}
}
