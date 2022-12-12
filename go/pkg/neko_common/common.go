package neko_common

import (
	"net/http"
)

var Version_v2ray string = "N/A"
var Version_neko string = "N/A"

var Debug bool

var RunMode int

const (
	RunMode_Other = iota
	RunMode_NekoRay_Core
	RunMode_NekoBox_Core
)

var GetProxyHttpClient func() *http.Client
