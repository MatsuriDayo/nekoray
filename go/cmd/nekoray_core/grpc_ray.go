package main

import (
	"context"
	"errors"
	"fmt"
	"grpc_server"
	"grpc_server/gen"
	"log"

	"github.com/matsuridayo/libneko/neko_common"
	"github.com/matsuridayo/libneko/speedtest"

	"github.com/xtls/xray-core/nekoutils"
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
		log.Println("EnableNekorayConnections:", in.EnableNekorayConnections)
	}

	if instance != nil {
		err = errors.New("instance already started")
		return
	}

	instance, err = NewNekoV2rayInstance(in.CoreConfig)
	if err != nil {
		return
	}

	nekoutils.SetConnectionPoolV2RayEnabled(instance.CorePtr(), in.EnableNekorayConnections)

	err = instance.Start()
	if err != nil {
		return
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

	err = instance.Close()
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

	if neko_common.Debug {
		log.Println("Test:", in)
	}

	if in.Mode == gen.TestMode_UrlTest {
		var i *NekoV2RayInstance

		if in.Config != nil {
			// Test instance
			i, err = NewNekoV2rayInstance(in.Config.CoreConfig)
			if err != nil {
				return
			}
			defer i.Close()

			err = i.Start()
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
		var t int32
		t, err = speedtest.UrlTest(createProxyHttpClient(i), in.Url, in.Timeout)
		out.Ms = t // sn: ms==0 是错误
	} else if in.Mode == gen.TestMode_TcpPing {
		out.Ms, err = speedtest.TcpPing(in.Address, in.Timeout)
	} else if in.Mode == gen.TestMode_FullTest {
		if in.Config == nil {
			return
		}
		// Test instance
		i, err := NewNekoV2rayInstance(in.Config.CoreConfig)
		if err != nil {
			return
		}
		defer i.Close()

		err = i.Start()
		if err != nil {
			return
		}

		return grpc_server.DoFullTest(ctx, in, i)
	}

	return
}

func (s *server) QueryStats(ctx context.Context, in *gen.QueryStatsReq) (out *gen.QueryStatsResp, _ error) {
	out = &gen.QueryStatsResp{}
	if instance != nil && instance.StatsManager != nil {
		counter := instance.StatsManager.GetCounter(fmt.Sprintf("outbound>>>%s>>>traffic>>>%s", in.Tag, in.Direct))
		if counter != nil {
			out.Traffic = counter.Set(0)
		}
	}
	return
}

func (s *server) ListConnections(ctx context.Context, in *gen.EmptyReq) (out *gen.ListConnectionsResp, _ error) {
	// nekoray_connections_json

	out = &gen.ListConnectionsResp{}
	if instance != nil {
		out.NekorayConnectionsJson = nekoutils.ListConnections(instance.CorePtr())
	}

	return
}
