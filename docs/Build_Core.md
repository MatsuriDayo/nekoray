## 构建 nekoray_core & nekobox_core

### 目录结构

```
  | nekoray
  |   go/cmd/*
  | Xray-core
  | sing-box-extra
  | sing-box
  | ......
```

### 常规构建

1. `bash libs/get_source.sh` （自动下载目录结构，自动 checkout commit）
2. `GOOS=windows GOARCH=amd64 bash libs/build_go.sh`

具体支持的 GOOS 和 GOARCH 请看 `libs/build_go.sh`

非官方构建无需编译 `updater` `launcher`

### sing-box tags

具体使用的 tags 请看 `libs/build_go.sh`
