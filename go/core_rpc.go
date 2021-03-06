package main

import (
	"context"
	"encoding/json"
	"errors"
	"fmt"
	"io"
	"libcore"
	"libcore/device"
	"libcore/stun"
	"nekoray_core/gen"
	"net"
	"os"
	"strings"
	"time"

	"github.com/sirupsen/logrus"
)

var instance *libcore.V2RayInstance
var setupCore_platforms = make([]func(), 0)

func setupCore() {
	device.IsNekoray = true
	libcore.SetConfig("", false, true)
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

	libcore.SetConfig(in.TryDomains, false, true)

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

		// Latency
		var t int32
		t, err = libcore.UrlTestV2ray(i, in.Inbound, in.Url, in.Timeout)
		out.Ms = t // sn: ms==0 ?????????
	} else if in.Mode == gen.TestMode_TcpPing {
		startTime := time.Now()
		_, err = net.DialTimeout("tcp", in.Address, time.Duration(in.Timeout)*time.Millisecond)
		endTime := time.Now()
		if err == nil {
			out.Ms = int32(endTime.Sub(startTime).Milliseconds())
		}
	} else if in.Mode == gen.TestMode_FullTest {
		if in.Config == nil {
			return
		}

		// Test instance
		i := libcore.NewV2rayInstance()
		defer i.Close()

		err = i.LoadConfig(in.Config.CoreConfig)
		if err != nil {
			return
		}

		err = i.Start()
		if err != nil {
			return
		}

		// Latency
		var latency string
		if in.FullLatency {
			t, _ := libcore.UrlTestV2ray(i, in.Inbound, in.Url, in.Timeout)
			out.Ms = t
			if t > 0 {
				latency = fmt.Sprint(t, "ms")
			} else {
				latency = "Error"
			}
		}

		// ?????? IP
		var in_ip string
		if in.FullInOut {
			_in_ip, err := net.ResolveIPAddr("ip", in.InAddress)
			if err == nil {
				in_ip = _in_ip.String()
			} else {
				in_ip = err.Error()
			}
		}

		client := getProxyHttpClient(i)

		// ?????? IP
		var out_ip string
		if in.FullInOut {
			resp, err := client.Get("https://httpbin.org/get")
			if err == nil {
				v := make(map[string]interface{})
				json.NewDecoder(resp.Body).Decode(&v)
				if a, ok := v["origin"]; ok {
					if s, ok := a.(string); ok {
						out_ip = s
					}
				}
				resp.Body.Close()
			} else {
				out_ip = "Error"
			}
		}

		// ??????
		var speed string
		if in.FullSpeed {
			resp, err := client.Get("http://cachefly.cachefly.net/10mb.test")
			if err == nil {
				time_start := time.Now()
				n, _ := io.Copy(io.Discard, resp.Body)
				time_end := time.Now()

				speed = fmt.Sprintf("%.2fMiB/s", (float64(n)/time_end.Sub(time_start).Seconds())/1048576)
				resp.Body.Close()
			} else {
				speed = "Error"
			}
		}

		// STUN
		var stunText string
		if in.FullNat {
			timeout := time.NewTimer(time.Second * 5)
			result := make(chan string, 0)

			go func() {
				stunServer := "206.53.159.130:3478"
				stunAddr, _ := net.ResolveUDPAddr("udp4", stunServer)
				pc, err := i.DialUDP(stunAddr)
				if err == nil {
					stunClient := stun.NewClientWithConnection(pc)
					stunClient.SetServerAddr(stunServer)
					nat, host, err, fake := stunClient.Discover()
					if err == nil {
						if host != nil {
							if fake {
								result <- fmt.Sprint("No Endpoint", nat)
							} else {
								result <- fmt.Sprint(nat)
							}
						}
					} else {
						result <- "Discover Error"
					}
				} else {
					result <- "DialUDP Error"
				}
				close(result)
			}()

			select {
			case <-timeout.C:
				stunText = "Timeout"
			case r := <-result:
				stunText = r
			}
		}

		fr := make([]string, 0)
		if latency != "" {
			fr = append(fr, fmt.Sprintf("Latency: %s", latency))
		}
		if speed != "" {
			fr = append(fr, fmt.Sprintf("Speed: %s", speed))
		}
		if in_ip != "" {
			fr = append(fr, fmt.Sprintf("In: %s", in_ip))
		}
		if out_ip != "" {
			fr = append(fr, fmt.Sprintf("Out: %s", out_ip))
		}
		if stunText != "" {
			fr = append(fr, fmt.Sprintf("NAT: %s", stunText))
		}

		out.FullReport = strings.Join(fr, " / ")
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

func (s *server) ListV2RayConnections(ctx context.Context, in *gen.EmptyReq) (*gen.ListV2RayConnectionsResp, error) {
	out := &gen.ListV2RayConnectionsResp{
		MatsuriConnectionsJson: libcore.ListV2rayConnections(),
	}
	return out, nil
}
