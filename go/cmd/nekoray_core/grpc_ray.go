package main

import (
	"context"
	"encoding/hex"
	"encoding/json"
	"errors"
	"fmt"
	"grpc_server"
	"grpc_server/gen"
	"io"
	"log"
	"net"
	"strings"
	"time"

	"github.com/matsuridayo/libneko/neko_common"
	"github.com/matsuridayo/libneko/speedtest"

	core "github.com/v2fly/v2ray-core/v5"
	"github.com/v2fly/v2ray-core/v5/nekoutils"
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
		t, err = speedtest.UrlTest(getProxyHttpClient(i), in.Url, in.Timeout)
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

		// Latency
		var latency string
		if in.FullLatency {
			t, _ := speedtest.UrlTest(getProxyHttpClient(i), in.Url, in.Timeout)
			out.Ms = t
			if t > 0 {
				latency = fmt.Sprint(t, "ms")
			} else {
				latency = "Error"
			}
		}

		// UDP Latency
		var udpLatency string
		if in.FullUdpLatency {
			ctx, cancel := context.WithTimeout(context.Background(), time.Second*3)
			result := make(chan string)

			go func() {
				var startTime = time.Now()
				pc, err := core.DialUDP(ctx, i.Instance)
				if err == nil {
					defer pc.Close()
					dnsPacket, _ := hex.DecodeString("0000010000010000000000000377777706676f6f676c6503636f6d0000010001")
					addr := &net.UDPAddr{
						IP:   net.ParseIP("8.8.8.8"),
						Port: 53,
					}
					_, err = pc.WriteTo(dnsPacket, addr)
					if err == nil {
						var buf [1400]byte
						_, _, err = pc.ReadFrom(buf[:])
					}
				}
				if err == nil {
					var endTime = time.Now()
					result <- fmt.Sprint(endTime.Sub(startTime).Abs().Milliseconds(), "ms")
				} else {
					result <- "Error"
				}
				close(result)
			}()

			select {
			case <-ctx.Done():
				udpLatency = "Timeout"
			case r := <-result:
				udpLatency = r
			}
			cancel()
		}

		// 入口 IP
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

		// 出口 IP
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

		// 下载
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

		fr := make([]string, 0)
		if latency != "" {
			fr = append(fr, fmt.Sprintf("Latency: %s", latency))
		}
		if udpLatency != "" {
			fr = append(fr, fmt.Sprintf("UDPLatency: %s", udpLatency))
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

		out.FullReport = strings.Join(fr, " / ")
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
