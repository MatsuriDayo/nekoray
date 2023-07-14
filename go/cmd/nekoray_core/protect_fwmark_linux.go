package main

import (
	"fmt"
	"log"
	"net"
	"strings"
	"syscall"

	"github.com/jsimonetti/rtnetlink"
	"github.com/xtls/xray-core/transport/internet"
	linuxcap "kernel.org/pub/linux/libs/security/libcap/cap"
)

var rtnetlink_conn *rtnetlink.Conn
var cap_net_admin = 0

func nekorayLinuxProtect(fd int) bool {
	if cap_net_admin == 0 {
		str := strings.ToLower(linuxcap.GetProc().String())
		if strings.Contains(str, "cap_net_admin") || str == "=ep" {
			cap_net_admin = 1
		} else {
			cap_net_admin = -1
		}
	}

	// check is in Tun Mode
	if is_fwmark_exist(514) {
		if cap_net_admin == 1 {
			// have permission
			if err := syscall.SetsockoptInt(fd, syscall.SOL_SOCKET, syscall.SO_MARK, 514); err != nil {
				log.Println("syscall.SetsockoptInt:", err)
				return false
			}
		} else {
			// don't have permission
			if err := cmsgProtect(fd, "./protect"); err != nil {
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
	internet.RegisterListenerController(func(network, address string, conn syscall.RawConn) error {
		conn.Control(func(fd uintptr) {
			nekorayLinuxProtect(int(fd))
		})
		return nil
	})
	internet.RegisterDialerController(func(network, address string, conn syscall.RawConn) error {
		conn.Control(func(fd uintptr) {
			nekorayLinuxProtect(int(fd))
		})
		return nil
	})
	underlyingNetDialer = &net.Dialer{
		Control: func(network, address string, c syscall.RawConn) error {
			c.Control(func(fd uintptr) {
				nekorayLinuxProtect(int(fd))
			})
			return nil
		},
	}
}
