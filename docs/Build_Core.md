## 构建 nekoray_core & nekobox_core

### 简易构建

1. 把 `Matsuridayo/v2ray-core` 置于 `../`
2. 进入 `go/cmd/nekoray_core` 文件夹 `go build` 得到 `nekoray_core`。
3. 进入 `go/cmd/nekobox_core` 文件夹 `go build` 得到 `nekobox_core`。

具体编译过程请参考 `libs/get_source.sh` `libs/build_go.sh`

非官方构建无需编译 `updater` `launcher`

### 常规构建

1. 把 `Matsuridayo/v2ray-core` 置于 `../`
2. `GOOS=windows GOARCH=amd64 bash libs/build_go.sh`

具体支持的 GOOS 和 GOARCH 请看 `libs/build_go.sh`

### sing-box tags

默认使用的 tags 以下变动外，其余均与官方版 sing-box 构建相同。具体使用的 tags 请看 `libs/build_go.sh`

1. 不包含 `reality_server`
2. `clash_api` 替换为 `v2ray_api`
