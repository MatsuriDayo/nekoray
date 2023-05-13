package main

import (
	"context"
	"net/http"

	"github.com/matsuridayo/libneko/neko_common"
	"github.com/matsuridayo/libneko/neko_log"
	"github.com/matsuridayo/sing-box-extra/boxapi"
	"github.com/matsuridayo/sing-box-extra/boxbox"
	"github.com/matsuridayo/sing-box-extra/boxmain"
)

var instance *boxbox.Box
var instance_cancel context.CancelFunc

func setupCore() {
	boxmain.DisableColor()
	//
	neko_log.SetupLog(50*1024, "./neko.log")
	neko_common.GetProxyHttpClient = func() *http.Client {
		return boxapi.CreateProxyHttpClient(instance)
	}
}
