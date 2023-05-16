package main

import (
	"flag"
	"io"
	"log"
	"net"
	"net/http"
	"os"
	"syscall"

	"github.com/jsimonetti/rtnetlink"
	linuxcap "kernel.org/pub/linux/libs/security/libcap/cap"
)

func ToolBox() {
	//
	var protectListenPath string
	var protectFwMark int
	//
	flag.StringVar(&protectListenPath, "protect-listen-path", "", "Set unix protect server listen path (Linux ROOT only)")
	flag.IntVar(&protectFwMark, "protect-fwmark", 0, "Set unix protect fwmark (Linux ROOT only)")
	flag.CommandLine.Parse(os.Args[3:])
	//
	switch os.Args[2] {
	case "rule":
		{
			// Dial a connection to the rtnetlink socket
			conn, err := rtnetlink.Dial(nil)
			if err != nil {
				log.Fatal(err)
			}
			defer conn.Close()

			// Request a list of rules
			rules, err := conn.Rule.List()
			if err != nil {
				log.Fatal(err)
			}

			for _, rule := range rules {
				log.Printf("%+v", rule)
				log.Printf("%+v", rule.Attributes)
			}

			for _, rule := range rules {
				if rule.Attributes.FwMark != nil {
					log.Printf("%+v", rule.Attributes)
					log.Println(*rule.Attributes.FwMark, *rule.Attributes.Table)
				}
			}
		}
	case "cap":
		{
			set := linuxcap.GetProc()
			if set != nil {
				log.Println(set)
			}
		}
	// case "protect":
	// 	{
	// 		if protectListenPath == "" {
	// 			log.Println("missing protect-listen-path")
	// 			return
	// 		}
	// 		protect_server.ServeProtect(protectListenPath, protectFwMark)
	// 	}
	case "btd": // Test the permission
		{
			d := &net.Dialer{
				Control: func(network, address string, c syscall.RawConn) (err error) {
					c.Control(func(fd uintptr) {
						err = syscall.BindToDevice(int(fd), os.Args[3])
					})
					return
				},
			}
			c := http.Client{
				Transport: &http.Transport{
					DialContext: d.DialContext,
				},
			}
			resp, err := c.Get(os.Args[4])
			if err != nil {
				log.Fatalln(err)
			}
			io.Copy(os.Stdout, resp.Body)
			resp.Body.Close()
		}
	}
}
