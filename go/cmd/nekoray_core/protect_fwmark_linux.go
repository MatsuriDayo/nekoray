package main

import (
	"fmt"
	"libcore/protect"
	"log"
	"net"
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

	// check is in VPN mode
	if is_fwmark_exist(514) {
		if cap_net_admin == 1 {
			if err := syscall.SetsockoptInt(int(fd), syscall.SOL_SOCKET, syscall.SO_MARK, 514); err != nil {
				log.Println("syscall.SetsockoptInt:", err)
				return false
			}
		} else {
			if err := cmsgProtect(int(fd), "./protect"); err != nil {
				log.Println("cmsgProtect:", err)
				return false
			}
		}
	}
	return true
}

func cmsgProtect(fd int, unixPath string) error {
	socket, err := syscall.Socket(syscall.AF_UNIX, syscall.SOCK_STREAM, 0)
	if err != nil {
		return err
	}
	defer syscall.Close(socket)

	syscall.SetsockoptTimeval(socket, syscall.SOL_SOCKET, syscall.SO_RCVTIMEO, &syscall.Timeval{Sec: 3})
	syscall.SetsockoptTimeval(socket, syscall.SOL_SOCKET, syscall.SO_SNDTIMEO, &syscall.Timeval{Sec: 3})

	err = syscall.Connect(socket, &syscall.SockaddrUnix{Name: unixPath})
	if err != nil {
		return err
	}

	err = syscall.Sendmsg(socket, nil, syscall.UnixRights(fd), nil, 0)
	if err != nil {
		return err
	}

	dummy := []byte{1}
	n, err := syscall.Read(socket, dummy)
	if err != nil {
		return err
	}
	if n != 1 {
		return fmt.Errorf("cmsgProtect protect failed")
	}
	return nil
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
	protect.FdProtector = &fwmarkProtector{}
	underlyingNetDialer = &net.Dialer{
		Control: func(network, address string, c syscall.RawConn) error {
			c.Control(func(fd uintptr) {
				protect.FdProtector.Protect(int32(fd))
			})
			return nil
		},
	}
}
