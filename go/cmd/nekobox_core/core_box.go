package main

import (
	"context"
	"net/http"

	"github.com/matsuridayo/libneko/neko_common"
	"github.com/matsuridayo/libneko/neko_log"
	"github.com/matsuridayo/sing-box-extra/boxapi"

	box "github.com/sagernet/sing-box"
)

var instance *box.Box
var instance_cancel context.CancelFunc

// Use sing-box instead of libcore & v2ray

func setupCore() {
	neko_log.SetupLog(50*1024, "./neko.log")
	//
	neko_common.GetProxyHttpClient = func() *http.Client {
		return boxapi.GetProxyHttpClient(instance)
	}
}
