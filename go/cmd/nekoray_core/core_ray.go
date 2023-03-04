package main

import (
	"context"
	"fmt"
	"log"
	"net"
	"net/http"
	"os"
	"time"

	"github.com/matsuridayo/libneko/neko_common"
	"github.com/matsuridayo/libneko/neko_log"

	core "github.com/v2fly/v2ray-core/v5"
	appLog "github.com/v2fly/v2ray-core/v5/app/log"
	commonLog "github.com/v2fly/v2ray-core/v5/common/log"
	v2rayNet "github.com/v2fly/v2ray-core/v5/common/net"
	"github.com/v2fly/v2ray-core/v5/features/dns"
	"github.com/v2fly/v2ray-core/v5/features/dns/localdns"
)

var instance *NekoV2RayInstance
var getNekorayTunIndex = func() int { return 0 } // Windows only
var underlyingNetDialer *net.Dialer

type v2rayLogWriter struct {
}

func (w *v2rayLogWriter) Write(s string) error {
	log.Print(s)
	return nil
}

func (w *v2rayLogWriter) Close() error {
	return nil
}

func setupCore() {
	// core setup
	os.Setenv("v2ray.conf.geoloader", "memconservative")
	neko_log.SetupLog(50*1024, "./neko.log")
	_ = appLog.RegisterHandlerCreator(appLog.LogType_Console, func(lt appLog.LogType,
		options appLog.HandlerCreatorOptions) (commonLog.Handler, error) {
		return commonLog.NewLogger(func() commonLog.Writer {
			return &v2rayLogWriter{}
		}), nil
	})
	// localdns setup
	resolver_def := &net.Resolver{PreferGo: false}
	resolver_go := &net.Resolver{PreferGo: true}
	if underlyingNetDialer != nil && os.Getenv("NKR_VPN_LEGACY_DNS") == "1" {
		resolver_def.Dial = underlyingNetDialer.DialContext
		resolver_go.Dial = underlyingNetDialer.DialContext
		log.Println("using NKR_VPN_LEGACY_DNS")
	}
	localdns.SetLookupFunc(func(network string, host string) (ips []net.IP, err error) {
		// fix old sekai
		defer func() {
			if len(ips) == 0 {
				log.Println("LookupIP error:", network, host, err)
				err = dns.ErrEmptyResponse
			}
		}()
		// Normal mode use system resolver (go bug)
		if getNekorayTunIndex() == 0 {
			return resolver_def.LookupIP(context.Background(), network, host)
		}
		// Windows VPN mode use Go resolver
		return resolver_go.LookupIP(context.Background(), network, host)
	})
	//
	neko_common.GetProxyHttpClient = func() *http.Client {
		return getProxyHttpClient(instance)
	}
}

// PROXY

func getProxyHttpClient(_instance *NekoV2RayInstance) *http.Client {
	dailContext := func(ctx context.Context, network, addr string) (net.Conn, error) {
		dest, err := v2rayNet.ParseDestination(fmt.Sprintf("%s:%s", network, addr))
		if err != nil {
			return nil, err
		}
		return core.Dial(ctx, _instance.Instance, dest)
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
