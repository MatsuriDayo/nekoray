package main

import (
	"errors"
	"libcore"
	"nekoray_core/gen"
	"sync"
	"syscall"

	gvisorTun "gvisor.dev/gvisor/pkg/tcpip/link/tun"
)

var tun2ray *libcore.Tun2ray
var tun_fd int
var tun_lock sync.Mutex

func TunStart(config *gen.SetTunReq) (err error) {
	tun_lock.Lock()
	defer tun_lock.Unlock()

	if tun2ray != nil {
		return errors.New("tun aleary started")
	}

	tun_fd, err = gvisorTun.Open(config.Name)
	if err != nil {
		return
	}

	tun2ray, err = libcore.NewTun2ray(&libcore.TunConfig{
		FileDescriptor: int32(tun_fd),
		MTU:            config.Mtu,
		V2Ray:          instance, // use current if started
		Implementation: config.Implementation,
		Sniffing:       true,
		FakeDNS:        config.Fakedns,
	})
	return
}

func TunStop() {
	tun_lock.Lock()
	defer tun_lock.Unlock()

	if tun2ray != nil {
		tun2ray.Close()
		tun2ray = nil
		if tun_fd > 0 {
			syscall.Close(tun_fd)
		}
		tun_fd = 0
	}
}

func TunSetV2ray(i *libcore.V2RayInstance) {
	tun_lock.Lock()
	defer tun_lock.Unlock()

	if tun2ray != nil {
		tun2ray.SetV2ray(i)
	}
}
