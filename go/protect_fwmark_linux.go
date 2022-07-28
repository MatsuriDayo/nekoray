package main

import (
	"libcore/protect"
	"log"
	"strings"
	"syscall"

	"github.com/jsimonetti/rtnetlink"
	linuxcap "kernel.org/pub/linux/libs/security/libcap/cap"
)

type fwmarkProtector struct{}

var rtnetlink_conn *rtnetlink.Conn
var cap_net_admin = 0

func (f *fwmarkProtector) Protect(fd int32) bool {
	if cap_net_admin == 0 {
		str := strings.ToLower(linuxcap.GetProc().String())
		if strings.Contains(str, "cap_net_admin") || str == "=ep" {
			cap_net_admin = 1
		} else {
			cap_net_admin = -1
		}
	}
	if cap_net_admin != 1 {
		//  no permission to set sockopt
		return true
	}

	// find bypass rule
	// TODO hardcoded fwmark 514
	if is_fwmark_exist(514) {
		if err := syscall.SetsockoptInt(int(fd), syscall.SOL_SOCKET, syscall.SO_MARK, 514); err != nil {
			log.Println("syscall.SetsockoptInt:", err.Error())
			return false
		}
	}
	return true
}

func is_fwmark_exist(number int) bool {
	var err error

	if rtnetlink_conn == nil {
		rtnetlink_conn, err = rtnetlink.Dial(nil)
		if err != nil {
			log.Println(err)
		}
		return false
	}

	rules, err := rtnetlink_conn.Rule.List()
	if err != nil {
		rtnetlink_conn = nil
		return false
	}

	for _, rule := range rules {
		if rule.Attributes != nil && rule.Attributes.FwMark != nil && uint32(number) == *rule.Attributes.FwMark {
			return true
		}
	}

	return false
}

func init() {
	setupCore_platforms = append(setupCore_platforms, func() {
		protect.FdProtector = &fwmarkProtector{}
	})
}
