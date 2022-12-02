package neko_common

import (
	"net/http"
	"strings"
	"time"
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

func GetBuildTime() int64 {
	if !strings.HasPrefix(Version_neko, "nekoray-") {
		return 0
	}
	ver := strings.TrimLeft(Version_neko, "nekoray-")
	buildDate := ver[strings.Index(ver, "-")+1:]
	buildTime, _ := time.Parse("2006-01-02", buildDate)
	return buildTime.Unix()
}

func GetExpireTime() int64 {
	buildTime := time.Unix(GetBuildTime(), 0)
	expireTime := buildTime.AddDate(0, 6, 0) // current force update: 6 months
	return expireTime.Unix()
}
