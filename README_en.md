# NekoRay

Qt based cross-platform GUI proxy configuration manager (backend: v2ray / sing-box)

Support Windows / Linux / macOS amd64 out of the box now.

基于 Qt 的跨平台代理配置管理器 (后端 v2ray / sing-box)

目前支持 Windows / Linux / macOS amd64 开箱即用

## 下载 / Download

### GitHub Releases (Portable ZIP)

便携格式，无安装器。转到 Releases 下载预编译的二进制文件，解压后即可使用。

[![GitHub All Releases](https://img.shields.io/github/downloads/Matsuridayo/nekoray/total?label=downloads-total&logo=github&style=flat-square)](https://github.com/Matsuridayo/nekoray/releases)

[Download](https://github.com/Matsuridayo/nekoray/releases)

### Package

1. AUR [nekoray-git](https://aur.archlinux.org/packages/nekoray-git)
2. Scoop extras: `scoop install nekoray`

## 更改记录 & 发布频道 / Changelog & Telegram channel

https://t.me/Matsuridayo

## 项目主页 & 文档 / Homepage & Documents

https://matsuridayo.github.io

## 代理 / Proxy

* SOCKS (4/4a/5)
* HTTP(S)
* Shadowsocks
* VMess
* VLESS
* Trojan
* NaïveProxy ( Custom Core )
* Hysteria ( Custom Core or sing-box )
* Custom Outbound
* Custom Core

## 订阅 / Subscription

* Raw: some widely used formats (like shadowsocks, clash and v2rayN)
* 原始格式：一些广泛使用的格式（如 shadowsocks、clash 和 v2rayN）

## 运行参数

- `-many` 无视同目录正在运行的实例，强行开启新的实例 (0.11+)
- `-appdata` 开启后配置文件会放在共享目录，无法多开和自动升级 (0.11+)

## Windows 运行

若提示 DLL 缺失，无法运行，请下载 安装 [微软 C++ 运行库](https://aka.ms/vs/17/release/vc_redist.x64.exe)

## Linux 运行

[Linux 运行教程](docs/Run_Linux.md)

## 编译教程 / Compile Tutorial

[Linux](docs/Build_Linux.md)

[Mac](docs/Build_MacOS.md)

[Windows](docs/Build_Windows.md)

## 捐助 / Donate

Donations are welcome to support project development.

欢迎捐赠以支持项目开发。

USDT TRC20

`TRhnA7SXE5Sap5gSG3ijxRmdYFiD4KRhPs`

XMR

`49bwESYQjoRL3xmvTcjZKHEKaiGywjLYVQJMUv79bXonGiyDCs8AzE3KiGW2ytTybBCpWJUvov8SjZZEGg66a4e59GXa6k5`

## Credits

- [v2fly/v2ray-core](https://github.com/v2fly/v2ray-core)
- [MatsuriDayo/Matsuri](https://github.com/MatsuriDayo/Matsuri)
- [MatsuriDayo/v2ray-core](https://github.com/MatsuriDayo/v2ray-core)
- [SagerNet/sing-box](https://github.com/SagerNet/sing-box)
- [Qv2ray](https://github.com/Qv2ray/Qv2ray)
- [Qt](https://www.qt.io/)
- [protobuf](https://github.com/protocolbuffers/protobuf)
- [yaml-cpp](https://github.com/jbeder/yaml-cpp)
- [zxing-cpp](https://github.com/nu-book/zxing-cpp)
- [QHotkey](https://github.com/Skycoder42/QHotkey)
