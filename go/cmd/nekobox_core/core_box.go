package main

import (
	"context"
	"net/http"

	"github.com/matsuridayo/libneko/neko_common"
	"github.com/matsuridayo/libneko/neko_log"
	"github.com/matsuridayo/sing-box-extra/boxapi"
	"github.com/matsuridayo/sing-box-extra/boxbox"
)

var instance *boxbox.Box
var instance_cancel context.CancelFunc

// Use sing-box instead of libcore & v2ray

func setupCore() {
	neko_log.SetupLog(50*1024, "./neko.log")
	//
	neko_common.GetProxyHttpClient = func() *http.Client {
		return boxapi.GetProxyHttpClient(instance)
	}
}
