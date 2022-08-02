package main

import (
	"encoding/binary"
	"log"
	"net"
	"strings"
	"unsafe"

	"github.com/v2fly/v2ray-core/v5/transport/internet"
	"golang.org/x/sys/windows"
)

func init() {
	internet.RegisterListenerController(func(network, address string, fd uintptr) error {
		bindInterfaceIndex := getBindInterfaceIndex()
		if bindInterfaceIndex != 0 {
			if err := bindInterface(fd, bindInterfaceIndex, true, true); err != nil {
				log.Println("bind inbound interface", err)
				return err
			}
		}
		return nil
	})
	internet.RegisterDialerController(func(network, address string, fd uintptr) error {
		bindInterfaceIndex := getBindInterfaceIndex()
		if bindInterfaceIndex != 0 {
			var v4, v6 bool
			if strings.HasSuffix(network, "6") {
				v4 = false
				v6 = true
			} else {
				v4 = true
				v6 = false
			}
			if err := bindInterface(fd, bindInterfaceIndex, v4, v6); err != nil {
				log.Println("bind outbound interface", err)
				return err
			}
		}
		return nil
	})
}

func getBindInterfaceIndex() uint32 {
	intfs, err := net.Interfaces()
	if err != nil {
		return 0
	}
	if len(intfs) > 1 {
		if intfs[0].Name == "nekoray-tun" || intfs[0].Name == "wintun" || intfs[0].Name == "TunMax" {
			return uint32(intfs[1].Index)
		}
	}
	return 0
}

const (
	IP_UNICAST_IF   = 31 // nolint: golint,stylecheck
	IPV6_UNICAST_IF = 31 // nolint: golint,stylecheck
)

func bindInterface(fd uintptr, interfaceIndex uint32, v4, v6 bool) error {
	if v4 {
		/* MSDN says for IPv4 this needs to be in net byte order, so that it's like an IP address with leading zeros. */
		bytes := make([]byte, 4)
		binary.BigEndian.PutUint32(bytes, interfaceIndex)
		interfaceIndex_v4 := *(*uint32)(unsafe.Pointer(&bytes[0]))

		if err := windows.SetsockoptInt(windows.Handle(fd), windows.IPPROTO_IP, IP_UNICAST_IF, int(interfaceIndex_v4)); err != nil {
			return err
		}
	}

	if v6 {
		if err := windows.SetsockoptInt(windows.Handle(fd), windows.IPPROTO_IPV6, IPV6_UNICAST_IF, int(interfaceIndex)); err != nil {
			return err
		}
	}

	return nil
}
