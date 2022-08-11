package main

import (
	"context"
	"encoding/json"
	"fmt"
	"io"
	"libcore"
	"nekoray_core/gen"
	"net"
	"net/http"
	"os"
	"runtime"
	"strings"
	"time"

	v2rayNet "github.com/v2fly/v2ray-core/v5/common/net"
)

// PROXY

func getProxyHttpClient(_instance *libcore.V2RayInstance) *http.Client {
	dailContext := func(ctx context.Context, network, addr string) (net.Conn, error) {
		dest, err := v2rayNet.ParseDestination(fmt.Sprintf("%s:%s", network, addr))
		if err != nil {
			return nil, err
		}
		return _instance.DialContext(ctx, dest)
	}

	transport := &http.Transport{
		TLSHandshakeTimeout:   time.Second * 3,
		ResponseHeaderTimeout: time.Second * 3,
	}
	if _instance != nil {
		transport.DialContext = dailContext
	}

	client := &http.Client{
		Transport: transport,
	}

	return client
}

// UPDATE

var update_download_url string

func (s *server) Update(ctx context.Context, in *gen.UpdateReq) (*gen.UpdateResp, error) {
	ret := &gen.UpdateResp{}

	client := getProxyHttpClient(instance)

	if in.Action == gen.UpdateAction_Check { // Check update
		ctx, cancel := context.WithTimeout(ctx, time.Second*10)
		defer cancel()

		req, _ := http.NewRequestWithContext(ctx, "GET", "https://api.github.com/repos/MatsuriDayo/nekoray/releases", nil)
		resp, err := client.Do(req)
		if err != nil {
			ret.Error = err.Error()
			return ret, nil
		}
		defer resp.Body.Close()

		v := []struct {
			HtmlUrl string `json:"html_url"`
			Assets  []struct {
				Name               string `json:"name"`
				BrowserDownloadUrl string `json:"browser_download_url"`
			} `json:"assets"`
			Prerelease bool   `json:"prerelease"`
			Body       string `json:"body"`
		}{}
		err = json.NewDecoder(resp.Body).Decode(&v)
		if err != nil {
			ret.Error = err.Error()
			return ret, nil
		}

		nowVer := strings.TrimLeft(version_standalone, "nekoray-")

		var search string
		if runtime.GOOS == "windows" && runtime.GOARCH == "amd64" {
			search = "windows64"
		} else if runtime.GOOS == "linux" && runtime.GOARCH == "amd64" {
			search = "linux64"
		} else {
			ret.Error = "Not official support platform"
			return ret, nil
		}

		for _, release := range v {
			if len(release.Assets) > 0 {
				for _, asset := range release.Assets {
					if strings.Contains(asset.Name, nowVer) {
						return ret, nil // No update
					}
					if strings.Contains(asset.Name, search) {
						if release.Prerelease {
							continue
						}
						update_download_url = asset.BrowserDownloadUrl
						ret.AssetsName = asset.Name
						ret.DownloadUrl = asset.BrowserDownloadUrl
						ret.ReleaseUrl = release.HtmlUrl
						ret.ReleaseNote = release.Body
						return ret, nil // update
					}
				}
			}
		}
	} else { // Download update
		if update_download_url == "" {
			ret.Error = "?"
			return ret, nil
		}

		req, _ := http.NewRequestWithContext(ctx, "GET", update_download_url, nil)
		resp, err := client.Do(req)
		if err != nil {
			ret.Error = err.Error()
			return ret, nil
		}
		defer resp.Body.Close()

		f, err := os.OpenFile("../nekoray.zip", os.O_TRUNC|os.O_CREATE|os.O_RDWR, 0644)
		if err != nil {
			ret.Error = err.Error()
			return ret, nil
		}
		defer f.Close()

		_, err = io.Copy(f, resp.Body)
		if err != nil {
			ret.Error = err.Error()
			return ret, nil
		}
		f.Sync()
	}

	return ret, nil
}
