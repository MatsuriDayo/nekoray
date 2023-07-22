package grpc_server

import (
	"context"
	"encoding/hex"
	"fmt"
	"grpc_server/gen"
	"io"
	"log"
	"net"
	"net/http"
	"strings"
	"time"

	"github.com/matsuridayo/libneko/neko_common"
	"github.com/matsuridayo/libneko/speedtest"
)

func getBetweenStr(str, start, end string) string {
	n := strings.Index(str, start)
	if n == -1 {
		n = 0
	}
	str = string([]byte(str)[n:])
	m := strings.Index(str, end)
	if m == -1 {
		m = len(str)
	}
	str = string([]byte(str)[:m])
	return str[len(start):]
}

func DoFullTest(ctx context.Context, in *gen.TestReq, instance interface{}) (out *gen.TestResp, _ error) {
	out = &gen.TestResp{}
	httpClient := neko_common.CreateProxyHttpClient(instance)

	// Latency
	var latency string
	if in.FullLatency {
		t, _ := speedtest.UrlTest(httpClient, in.Url, in.Timeout)
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
			pc, err := neko_common.DialContext(ctx, instance, "udp", "8.8.8.8:53")
			if err == nil {
				defer pc.Close()
				dnsPacket, _ := hex.DecodeString("0000010000010000000000000377777706676f6f676c6503636f6d0000010001")
				_, err = pc.Write(dnsPacket)
				if err == nil {
					var buf [1400]byte
					_, err = pc.Read(buf[:])
				}
			}
			if err == nil {
				var endTime = time.Now()
				result <- fmt.Sprint(endTime.Sub(startTime).Abs().Milliseconds(), "ms")
			} else {
				log.Println("UDP Latency test error:", err)
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

	// 出口 IP
	var out_ip string
	if in.FullInOut {
		resp, err := httpClient.Get("https://www.cloudflare.com/cdn-cgi/trace")
		if err == nil {
			b, _ := io.ReadAll(resp.Body)
			out_ip = getBetweenStr(string(b), "ip=", "\n")
			resp.Body.Close()
		} else {
			out_ip = "Error"
		}
	}

	// 下载
	var speed string
	if in.FullSpeed {
		if in.FullSpeedTimeout <= 0 {
			in.FullSpeedTimeout = 30
		}

		ctx, cancel := context.WithTimeout(context.Background(), time.Second*time.Duration(in.FullSpeedTimeout))
		result := make(chan string)
		var bodyClose io.Closer

		go func() {
			req, _ := http.NewRequestWithContext(ctx, "GET", in.FullSpeedUrl, nil)
			resp, err := httpClient.Do(req)
			if err == nil && resp != nil && resp.Body != nil {
				bodyClose = resp.Body
				defer resp.Body.Close()
				//
				time_start := time.Now()
				n, _ := io.Copy(io.Discard, resp.Body)
				time_end := time.Now()
				result <- fmt.Sprintf("%.2fMiB/s", (float64(n)/time_end.Sub(time_start).Seconds())/1048576)
			} else {
				result <- "Error"
			}
			close(result)
		}()

		select {
		case <-ctx.Done():
			speed = "Timeout"
		case s := <-result:
			speed = s
		}

		cancel()
		if bodyClose != nil {
			bodyClose.Close()
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

	return
}
