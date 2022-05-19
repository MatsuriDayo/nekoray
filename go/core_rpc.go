package main

import (
	"context"
	"errors"
	"libcore"
	"libcore/device"
	"nekoray_core/gen"
	"net"
	"os"
	"time"

	"github.com/sirupsen/logrus"
)

var instance *libcore.V2RayInstance
var setupCore_platforms = make([]func(), 0)

func setupCore() {
	device.IsNekoray = true
	libcore.InitCore("", "", "", nil, ".", "moe.nekoray.pc:bg", true, 50)
	for _, f := range setupCore_platforms {
		f()
	}
}

func (s *server) Start(ctx context.Context, in *gen.LoadConfigReq) (out *gen.ErrorResp, _ error) {
	var err error

	// only error use this
	defer func() {
		out = &gen.ErrorResp{}
		if err != nil {
			out.Error = err.Error()
			instance = nil
		}
	}()

	if nekoray_debug {
		logrus.Println("Start:", in.CoreConfig)
	}

	if instance != nil {
		err = errors.New("Already started...")
		return
	}

	instance = libcore.NewV2rayInstance()

	err = instance.LoadConfig(in.CoreConfig)
	if err != nil {
		return
	}

	err = instance.Start()
	if err != nil {
		return
	}

	return
}

func (s *server) Stop(ctx context.Context, in *gen.EmptyReq) (out *gen.ErrorResp, _ error) {
	var err error

	// only error use this
	defer func() {
		out = &gen.ErrorResp{}
		if err != nil {
			out.Error = err.Error()
		}
	}()

	if instance != nil {
		err = instance.Close()
		instance = nil
	}

	return
}

func (s *server) Exit(ctx context.Context, in *gen.EmptyReq) (out *gen.EmptyResp, _ error) {
	out = &gen.EmptyResp{}

	// Connection closed
	os.Exit(0)
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

	if nekoray_debug {
		logrus.Println("Test:", in)
	}

	if in.Mode == gen.TestMode_UrlTest {
		var i *libcore.V2RayInstance

		if in.Config != nil {
			// Test instance
			i = libcore.NewV2rayInstance()
			defer i.Close()

			err = i.LoadConfig(in.Config.CoreConfig)
			if err != nil {
				return
			}

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

		var t int32
		t, err = libcore.UrlTestV2ray(i, in.Inbound, in.Url, in.Timeout)
		out.Ms = t // sn: ms==0 是错误
	} else { // TCP Ping
		startTime := time.Now()
		_, err = net.DialTimeout("tcp", in.Address, time.Duration(in.Timeout)*time.Millisecond)
		endTime := time.Now()
		if err == nil {
			out.Ms = int32(endTime.Sub(startTime).Milliseconds())
		}
	}

	return
}

func (s *server) QueryStats(ctx context.Context, in *gen.QueryStatsReq) (out *gen.QueryStatsResp, _ error) {
	out = &gen.QueryStatsResp{}
	if instance != nil {
		out.Traffic = instance.QueryStats(in.Tag, in.Direct)
	}
	return
}
