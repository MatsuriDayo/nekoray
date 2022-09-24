package main

import (
	"context"
	"errors"
	"log"
	"neko/gen"
	"neko/pkg/grpc_server"
	"neko/pkg/neko_common"
	"nekobox_core/box_main"
	"net"
	"time"

	box "github.com/sagernet/sing-box"
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

	instance, instance_cancel, err = box_main.Create([]byte(in.CoreConfig), true)
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

	instance_cancel()
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

	if in.Mode == gen.TestMode_UrlTest {
		var i *box.Box
		if in.Config != nil {
			// Test instance
			instance, instance_cancel, err = box_main.Create([]byte(in.Config.CoreConfig), true)
			if instance_cancel != nil {
				defer instance_cancel()
			}
			if err != nil {
				return
			}
			i = instance
		} else {
			// Test running instance
			i = instance
			if i == nil {
				return
			}
		}
		// Latency
		out.Ms, err = UrlTestSingBox(i, in.Url, in.Timeout)
	} else if in.Mode == gen.TestMode_TcpPing {
		host, port, err := net.SplitHostPort(in.Address)
		if err != nil {
			out.Error = err.Error()
			return
		}
		ip, err := net.ResolveIPAddr("ip", host)
		if err != nil {
			out.Error = err.Error()
			return
		}
		//
		startTime := time.Now()
		_, err = net.DialTimeout("tcp", net.JoinHostPort(ip.String(), port), time.Duration(in.Timeout)*time.Millisecond)
		endTime := time.Now()
		if err == nil {
			out.Ms = int32(endTime.Sub(startTime).Milliseconds())
		}
	} else {
		// TODO copy
	}

	return
}

func (s *server) QueryStats(ctx context.Context, in *gen.QueryStatsReq) (out *gen.QueryStatsResp, _ error) {
	out = &gen.QueryStatsResp{}
	// TODO upstream api
	return
}

func (s *server) ListConnections(ctx context.Context, in *gen.EmptyReq) (*gen.ListConnectionsResp, error) {
	out := &gen.ListConnectionsResp{
		// TODO upstream api
	}
	return out, nil
}
