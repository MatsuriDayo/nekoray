# NekoRay

基于 Qt/C++ 的跨平台代理配置管理器（ 使用 Matsuri 定制版 v2ray-core ）

Qt/C++ based cross-platform proxy configuration manager ( Use Matsuri custom version of v2ray-core )

## 下载

便携格式，无安装器。转到 Releases 下载预编译的二进制文件，解压后即可使用。

## 进度

功能逐渐完善中。

✅ = 已完成

❌ = 未完成

❌❌ = 暂无计划

❓ = 未知 or 半成品

### 代理

| 协议           | 状态     | 配置编辑 | 分享链接生成 | 分享链接解析    | Clash 配置解析 |
|--------------|--------|------|--------|-----------|------------|
| Socks        | ✅      | ✅    | ✅      | ✅         | ✅          | 
| HTTP         | ✅      | ✅    | ✅      | ✅         | ✅          | 
| Shadowsocks  | ✅ (经典) | ✅    | ✅      | 常见格式      | ✅          |
| ShadowsocksR | ❌❌     |      |        |           |            |
| VMess        | ✅      | ✅    | ✅      | v2rayN 格式 | ✅          |
| Trojan       | ✅      | ✅    | ✅      | 标准&常见格式   | ✅          |
| VLESS        | ✅      | ✅    | ✅      | ✅         | 不适用        |
| NaïveProxy   | ✅      | ✅    | ❌      | ❌         | 不适用        |
| Hysteria     | ✅      | ❌    | ❌      | ❌         | 不适用        |

### 功能

| 功能                            | 状态                |
|-------------------------------|-------------------|
| (协议类↓)                        |                   |
| socks/http 入站                 | ✅                 |
| 自定义 v2ray 入站                  | ✅                 |
| v2ray mux.cool                | ✅                 |
| 自定义 v2ray stream settings     | ✅                 |
| 自定义 v2ray outbound object     | ✅                 |
| 外部 socks 接入 (类似 SagerNet 的插件) | ✅                 |
| 自定义配置接入 (类似 Matsuri 的高级插件)    | ❓                 |
| 自定义 v2ray 路由                  | ✅                 |
| “多节点分流”                       | ❌❌                |
| “负载均衡”                        | ❌❌                |
| (实用类↓)                        |                   |
| 日志显示                          | ✅                 |
| 连接显示                          | ✅                 |
| 搜索配置                          | ✅                 |
| 分组                            | ✅                 |
| 订阅                            | ❓                 |
| 剪切板导入                         | ✅                 |
| QR Code 扫描                    | ✅                 |
| 剪切板导出                         | ✅                 |
| QR Code 生成                    | ✅                 |
| 流量/速度统计                       | ✅                 |
| 链式代理                          | ✅                 |
| TCPing/URLTest                | ✅                 |
| 中文界面                          | ✅                 |
| (系统类↓)                        |                   |
| 平台可用性                         | 64位 Linux/Windows |
| 预编译 amd64 Linux 二进制文件         | ✅ (不能用则需自己编译)     |
| 预编译 amd64 Windows 二进制文件       | ✅                 |
| 系统代理                          | Linux/Windows     |
| 系统代理分流                        | ❌❌                |
| VPN on Linux                  | ❓                 |
| VPN on Windows                | ❌❌                |
| 检查更新                          | ✅                 |
| 主题样式                          | ✅                 |

## Linux 运行 & 简易编译教程

| CMake 参数         | 默认值 | 含义                      | 
|------------------|-----|-------------------------|
| QT_VERSION_MAJOR | 5   | QT版本                    | 
| NKR_NO_EXTERNAL  |     | 不包含外部C++依赖(如ZXing/gRPC) | 
| NKR_NO_GRPC      |     | 不包含gRPC                 |

**使用 Linux 系统相信您已具备基本的排错能力，本项目不提供特定发行版/架构的支持，预编译文件不能满足您的需求时，请自行编译/适配。**

系统要求： Qt5 运行环境，一般桌面 Linux 已经安装，如果没有请用包管理器安装，如 `apt install libqt5gui5`

运行： `./launcher`

已知部分 Linux 系统上使用发布的二进制文件，进入主界面后容易闪退，可以尝试自行编译。

### 简单编译法

C++ 依赖： `qt5 grpc yaml-cpp zxing-cpp` 包管理器安装， Qt 版本必须大于等于 5.15

```
mkdir build
cd build
cmake ..
make -j
```

编译完成后得到 `nekoray`

解压 Release 的压缩包，替换其中的 `nekoray`，删除 `launcher` 即可使用。

### 复杂编译法

C++ 部分

当您的发行版没有上面几个 C++ 依赖包，或者版本不符合要求时，可以参考 libs 文件夹内的默认编译脚本自行编译。

依赖搜寻 prefix 为 `libs/deps/bulit`

编译完成后得到 `nekoray`

Go 部分

把 `Matsuridayo/Matsuri` `Matsuridayo/v2ray-core` 置于 `../` ，进入 `go` 文件夹 `go build` 得到 `nekoray_core`。

非官方构建无需编译 `update` `launcher`

## Credits

Nekoray 的诞生离不开 v2ray 开源项目以及其他开源软件。

依赖:

- [v2fly/v2ray-core](https://github.com/v2fly/v2ray-core)
- [MatsuriDayo/Matsuri](https://github.com/MatsuriDayo/Matsuri)
- Qt
- gRPC
- yaml-cpp
- zxing-cpp

参考：

- [Qv2ray/Qv2ray](https://github.com/Qv2ray/Qv2ray)
- [2dust/v2rayN](https://github.com/2dust/v2rayN)
- [SagerNet/SagerNet](https://github.com/SagerNet/SagerNet)
