package main

import (
	"context"
	"fmt"
	"libcore"
	"libcore/device"
	"neko/pkg/neko_common"
	"net"
	"net/http"
	"time"

	v2rayNet "github.com/v2fly/v2ray-core/v5/common/net"
)

var instance *libcore.V2RayInstance

func setupCore() {
	// TODO del
	device.IsNekoray = true
	libcore.SetConfig("", false, true)
	libcore.InitCore("", "", "", nil, ".", "moe.nekoray.pc:bg", true, 50)
	//
	neko_common.GetProxyHttpClient = func() *http.Client {
		return getProxyHttpClient(instance)
	}
}

// PROXY

func getProxyHttpClient(_instance *libcore.V2RayInstance) *http.Client {
	dailContext := func(ctx context.Context, network, addr string) (net.Conn, error) {
		dest, err := v2rayNet.ParseDestination(fmt.Sprintf("%s:%s", network, addr))
		if err != nil {
			return nil, err
		}
		return _instance.DialContext(ctx, dest)
	}

	transport := &http.Transport{
		TLSHandshakeTimeout:   time.Second * 3,
		ResponseHeaderTimeout: time.Second * 3,
	}

	if _instance != nil {
		transport.DialContext = dailContext
	}

	client := &http.Client{
		Transport: transport,
	}

	return client
}
