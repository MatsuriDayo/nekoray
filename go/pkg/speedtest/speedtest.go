package speedtest

import (
	"context"
	"fmt"
	"log"
	"math/rand"
	"net"
	"net/http"
	"strings"
	"time"
)

func UrlTest(client *http.Client, link string, timeout int32) (int32, error) {
	if client == nil {
		return 0, fmt.Errorf("no client")
	}

	// Test handshake time
	var time_start time.Time
	var times = 1
	var rtt_times = 1

	// Test RTT "true delay"
	if link2 := strings.TrimLeft(link, "true"); link != link2 {
		link = link2
		times = 3
		rtt_times = 2
	}

	ctx, cancel := context.WithTimeout(context.Background(), time.Duration(timeout)*time.Millisecond)
	defer cancel()

	req, err := http.NewRequestWithContext(ctx, "GET", link, nil)
	req.Header.Set("User-Agent", fmt.Sprintf("curl/7.%d.%d", rand.Int()%84, rand.Int()%2))
	if err != nil {
		return 0, err
	}

	for i := 0; i < times; i++ {
		if i == 1 || times == 1 {
			time_start = time.Now()
		}

		resp, err := client.Do(req)
		if err != nil {
			log.Println("Url test failed:", err)
			return 0, err
		}
		resp.Body.Close()
	}

	return int32(time.Since(time_start).Milliseconds() / int64(rtt_times)), nil
}

func TcpPing(address string, timeout int32) (ms int32, err error) {
	startTime := time.Now()
	c, err := net.DialTimeout("tcp", address, time.Duration(timeout)*time.Millisecond)
	endTime := time.Now()
	if err == nil {
		ms = int32(endTime.Sub(startTime).Milliseconds())
		c.Close()
	}
	return
}
