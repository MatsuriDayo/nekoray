package v2ray_sip

import (
	"context"

	obfs "github.com/Dreamacro/clash/transport/simple-obfs"
	"github.com/xtls/xray-core/proxy/shadowsocks"
	"github.com/xtls/xray-core/transport/internet"
)

var _ shadowsocks.StreamPlugin = (*obfsLocalPlugin)(nil)

func init() {
	shadowsocks.RegisterPlugin("obfs-local", func() shadowsocks.SIP003Plugin {
		return new(obfsLocalPlugin)
	})
}

type obfsLocalPlugin struct {
	tls  bool
	host string
	port string
}

func (p *obfsLocalPlugin) Init(_ context.Context, _ string, _ string, _ string, remotePort string, pluginOpts string, _ []string, _ *shadowsocks.MemoryAccount) error {
	options, err := ParsePluginOptions(pluginOpts)
	if err != nil {
		return newError("obfs-local: failed to parse plugin options").Base(err)
	}

	mode := "http"

	if s, ok := options.Get("obfs"); ok {
		mode = s
	}

	if s, ok := options.Get("obfs-host"); ok {
		p.host = s
	}

	switch mode {
	case "http":
	case "tls":
		p.tls = true
	default:
		return newError("unknown obfs mode ", mode)
	}

	p.port = remotePort

	return nil
}

func (p *obfsLocalPlugin) StreamConn(connection internet.Connection) internet.Connection {
	if !p.tls {
		return obfs.NewHTTPObfs(connection, p.host, p.port)
	} else {
		return obfs.NewTLSObfs(connection, p.host)
	}
}

func (p *obfsLocalPlugin) Close() error {
	return nil
}
