package main

import (
	"context"
	"fmt"
	"log"
	"math/rand"
	"neko/pkg/neko_common"
	"net"
	"net/http"
	"os"
	"reflect"
	"strings"
	"time"
	"unsafe"

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
	log.SetFlags(log.LstdFlags)
	log.SetOutput(os.Stdout)
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

// TODO move
func UrlTestSingBox(box *box.Box, link string, timeout int32) (int32, error) {
	client := getProxyHttpClient(box)
	if client == nil {
		return 0, fmt.Errorf("no client")
	}

	// Test handshake time
	var time_start time.Time
	var times = 1
	var rtt_times = 1

	// Test RTT "true delay"
	if link2 := strings.TrimLeft(link, "true"); link != link2 {
		link = link2
		times = 3
		rtt_times = 2
	}

	ctx, cancel := context.WithTimeout(context.Background(), time.Duration(timeout)*time.Millisecond)
	defer cancel()
	req, err := http.NewRequestWithContext(ctx, "GET", link, nil)
	req.Header.Set("User-Agent", fmt.Sprintf("curl/7.%d.%d", rand.Int()%84, rand.Int()%2))
	if err != nil {
		return 0, err
	}

	for i := 0; i < times; i++ {
		if i == 1 || times == 1 {
			time_start = time.Now()
		}

		resp, err := client.Do(req)
		if err != nil {
			fmt.Println("Url test failed:", err)
			return 0, err
		}
		resp.Body.Close()
	}

	return int32(time.Since(time_start).Milliseconds() / int64(rtt_times)), nil
}
