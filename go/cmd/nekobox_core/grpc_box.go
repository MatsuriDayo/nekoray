package main

import (
	"context"
	"errors"
	"fmt"
	"time"

	"grpc_server"
	"grpc_server/gen"

	"github.com/matsuridayo/libneko/neko_common"
	"github.com/matsuridayo/libneko/neko_log"
	"github.com/matsuridayo/libneko/speedtest"
	"github.com/matsuridayo/sing-box-extra/boxapi"
	"github.com/matsuridayo/sing-box-extra/boxbox"
	"github.com/matsuridayo/sing-box-extra/boxmain"

	"log"

	"github.com/sagernet/sing-box/option"
)

type server struct {
	grpc_server.BaseServer
}

func (s *server) Start(ctx context.Context, in *gen.LoadConfigReq) (out *gen.ErrorResp, _ error) {
	var err error

	defer func() {
		out = &gen.ErrorResp{}
		if err != nil {
			out.Error = err.Error()
			instance = nil
		}
	}()

	if neko_common.Debug {
		log.Println("Start:", in.CoreConfig)
	}

	if instance != nil {
		err = errors.New("instance already started")
		return
	}

	instance, instance_cancel, err = boxmain.Create([]byte(in.CoreConfig))

	if instance != nil {
		// Logger
		instance.SetLogWritter(neko_log.LogWriter)
		// V2ray Service
		if in.StatsOutbounds != nil {
			instance.Router().SetV2RayServer(boxapi.NewSbV2rayServer(option.V2RayStatsServiceOptions{
				Enabled:   true,
				Outbounds: in.StatsOutbounds,
			}))
		}
	}

	return
}

func (s *server) Stop(ctx context.Context, in *gen.EmptyReq) (out *gen.ErrorResp, _ error) {
	var err error

	defer func() {
		out = &gen.ErrorResp{}
		if err != nil {
			out.Error = err.Error()
		}
	}()

	if instance == nil {
		return
	}

	instance.CloseWithTimeout(instance_cancel, time.Second*2, log.Println)

	instance = nil

	return
}

func (s *server) Test(ctx context.Context, in *gen.TestReq) (out *gen.TestResp, _ error) {
	var err error
	out = &gen.TestResp{Ms: 0}

	defer func() {
		if err != nil {
			out.Error = err.Error()
		}
	}()

	if in.Mode == gen.TestMode_UrlTest {
		var i *boxbox.Box
		var cancel context.CancelFunc
		if in.Config != nil {
			// Test instance
			i, cancel, err = boxmain.Create([]byte(in.Config.CoreConfig))
			if i != nil {
				defer i.Close()
				defer cancel()
			}
			if err != nil {
				return
			}
		} else {
			// Test running instance
			i = instance
			if i == nil {
				return
			}
		}
		// Latency
		out.Ms, err = speedtest.UrlTest(boxapi.CreateProxyHttpClient(i), in.Url, in.Timeout)
	} else if in.Mode == gen.TestMode_TcpPing {
		out.Ms, err = speedtest.TcpPing(in.Address, in.Timeout)
	} else if in.Mode == gen.TestMode_FullTest {
		i, cancel, err := boxmain.Create([]byte(in.Config.CoreConfig))
		if i != nil {
			defer i.Close()
			defer cancel()
		}
		if err != nil {
			return
		}
		return grpc_server.DoFullTest(ctx, in, i)
	}

	return
}

func (s *server) QueryStats(ctx context.Context, in *gen.QueryStatsReq) (out *gen.QueryStatsResp, _ error) {
	out = &gen.QueryStatsResp{}

	if instance != nil {
		if ss, ok := instance.Router().V2RayServer().(*boxapi.SbV2rayServer); ok {
			out.Traffic = ss.QueryStats(fmt.Sprintf("outbound>>>%s>>>traffic>>>%s", in.Tag, in.Direct))
		}
	}

	return
}

func (s *server) ListConnections(ctx context.Context, in *gen.EmptyReq) (*gen.ListConnectionsResp, error) {
	out := &gen.ListConnectionsResp{
		// TODO upstream api
	}
	return out, nil
}
