package main

import (
	"context"
	"io"
	"net"
	"net/http"
	"reflect"
	"time"
	"unsafe"

	"github.com/matsuridayo/libneko/neko_common"
	"github.com/matsuridayo/libneko/neko_log"

	box "github.com/sagernet/sing-box"
	"github.com/sagernet/sing-box/adapter"
	"github.com/sagernet/sing-box/common/dialer"
	"github.com/sagernet/sing/common/metadata"
	"github.com/sagernet/sing/common/network"
)

var instance *box.Box
var instance_cancel context.CancelFunc

// Use sing-box instead of libcore & v2ray

func setupCore() {
	neko_log.SetupLog(50*1024, "./neko.log")
	//
	neko_common.GetProxyHttpClient = func() *http.Client {
		return getProxyHttpClient(instance)
	}
}

func getProxyHttpClient(box *box.Box) *http.Client {
	var d network.Dialer

	if box != nil {
		router_ := reflect.Indirect(reflect.ValueOf(box)).FieldByName("router")
		router_ = reflect.NewAt(router_.Type(), unsafe.Pointer(router_.UnsafeAddr())).Elem()
		if router, ok := router_.Interface().(adapter.Router); ok {
			d = dialer.NewRouter(router)
		}
	}

	dialContext := func(ctx context.Context, network, addr string) (net.Conn, error) {
		return d.DialContext(ctx, network, metadata.ParseSocksaddr(addr))
	}

	transport := &http.Transport{
		TLSHandshakeTimeout:   time.Second * 3,
		ResponseHeaderTimeout: time.Second * 3,
	}

	if d != nil {
		transport.DialContext = dialContext
	}

	client := &http.Client{
		Transport: transport,
	}

	return client
}

type logWriter struct {
	files []io.Writer
}

func (w *logWriter) Write(p []byte) (n int, err error) {
	for _, file := range w.files {
		if file == nil {
			continue
		}
		n, err = file.Write(p)
		if err != nil {
			return
		}
	}
	return
}
