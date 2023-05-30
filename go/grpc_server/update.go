package grpc_server

import (
	"context"
	"encoding/json"
	"grpc_server/gen"
	"io"
	"net/http"
	"os"
	"runtime"
	"strings"
	"time"

	"github.com/matsuridayo/libneko/neko_common"
)

var update_download_url string

func (s *BaseServer) Update(ctx context.Context, in *gen.UpdateReq) (*gen.UpdateResp, error) {
	ret := &gen.UpdateResp{}

	client := neko_common.CreateProxyHttpClient(neko_common.GetCurrentInstance())

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

		nowVer := strings.TrimLeft(neko_common.Version_neko, "nekoray-")

		var search string
		if runtime.GOOS == "windows" && runtime.GOARCH == "amd64" {
			search = "windows64"
			// check Qt5 update after nekoray v3.3
			if _, err := os.Stat("../Qt5Core.dll"); err == nil {
				search = "windows7-x64"
			}
		} else if runtime.GOOS == "linux" && runtime.GOARCH == "amd64" {
			search = "linux64"
		} else if runtime.GOOS == "darwin" {
			search = "macos-" + runtime.GOARCH
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
						if release.Prerelease && !in.CheckPreRelease {
							continue
						}
						update_download_url = asset.BrowserDownloadUrl
						ret.AssetsName = asset.Name
						ret.DownloadUrl = asset.BrowserDownloadUrl
						ret.ReleaseUrl = release.HtmlUrl
						ret.ReleaseNote = release.Body
						ret.IsPreRelease = release.Prerelease
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
