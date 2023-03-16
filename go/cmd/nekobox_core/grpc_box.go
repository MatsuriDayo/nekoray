package main

import (
	"context"
	"errors"
	"fmt"

	"grpc_server"
	"grpc_server/gen"

	"github.com/matsuridayo/libneko/neko_common"
	"github.com/matsuridayo/libneko/neko_log"
	"github.com/matsuridayo/libneko/speedtest"
	"github.com/matsuridayo/sing-box-extra/boxapi"
	"github.com/matsuridayo/sing-box-extra/boxbox"
	"github.com/matsuridayo/sing-box-extra/boxmain"

	"io"
	"log"
	"reflect"
	"time"
	"unsafe"

	"github.com/sagernet/sing-box/experimental/v2rayapi"
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

	instance, instance_cancel, err = boxmain.Create([]byte(in.CoreConfig), true)

	if instance != nil {
		// Logger
		logFactory_ := reflect.Indirect(reflect.ValueOf(instance)).FieldByName("logFactory")
		logFactory_ = reflect.NewAt(logFactory_.Type(), unsafe.Pointer(logFactory_.UnsafeAddr())).Elem() // get unexported logFactory
		logFactory_ = logFactory_.Elem().Elem()                                                          // get struct
		writer_ := logFactory_.FieldByName("writer")
		writer_ = reflect.NewAt(writer_.Type(), unsafe.Pointer(writer_.UnsafeAddr())).Elem() // get unexported io.Writer
		writer_.Set(reflect.ValueOf(neko_log.LogWriter))
		// V2ray Service
		instance.Router().SetV2RayServer(boxapi.NewSbV2rayServer(option.V2RayStatsServiceOptions{
			Enabled:   true,
			Outbounds: []string{"proxy", "bypass"},
		}))
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

	t := time.NewTimer(time.Second * 2)
	c := make(chan struct{}, 1)

	go func(cancel context.CancelFunc, closer io.Closer) {
		cancel()
		closer.Close()
		c <- struct{}{}
		close(c)
	}(instance_cancel, instance)

	select {
	case <-t.C:
		log.Println("[Warning] sing-box close takes longer than expected.")
	case <-c:
	}

	t.Stop()
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
		if in.Config != nil {
			// Test instance
			i, instance_cancel, err = boxmain.Create([]byte(in.Config.CoreConfig), true)
			if instance_cancel != nil {
				defer instance_cancel()
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
		out.Ms, err = speedtest.UrlTest(boxapi.GetProxyHttpClient(i), in.Url, in.Timeout)
	} else if in.Mode == gen.TestMode_TcpPing {
		out.Ms, err = speedtest.TcpPing(in.Address, in.Timeout)
	} else {
		err = fmt.Errorf("not available")
	}

	return
}

func (s *server) QueryStats(ctx context.Context, in *gen.QueryStatsReq) (out *gen.QueryStatsResp, _ error) {
	out = &gen.QueryStatsResp{}

	var box_v2ray_service *boxapi.SbV2rayStatsService

	if instance != nil && instance.Router().V2RayServer() != nil {
		box_v2ray_service, _ = instance.Router().V2RayServer().StatsService().(*boxapi.SbV2rayStatsService)
	}

	if box_v2ray_service != nil {
		req := &v2rayapi.GetStatsRequest{
			Name:   fmt.Sprintf("outbound>>>%s>>>traffic>>>%s", in.Tag, in.Direct),
			Reset_: true,
		}
		resp, err := box_v2ray_service.GetStats(ctx, req)
		if err == nil {
			out.Traffic = resp.Stat.Value
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
