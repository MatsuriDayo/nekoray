package main

import (
	"flag"
	"log"

	"github.com/jsimonetti/rtnetlink"
	linuxcap "kernel.org/pub/linux/libs/security/libcap/cap"
)

func ToolBox(tool []string) {
	flag.Parse()

	switch tool[0] {
	case "tool":
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
	}
}
