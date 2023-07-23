# NekoRay / NekoBox For PC

Qt based cross-platform GUI proxy configuration manager (backend: v2ray / sing-box)

Support Windows / Linux out of the box now.

基于 Qt 的跨平台代理配置管理器 (后端 v2ray / sing-box)

目前支持 Windows / Linux 开箱即用

## Readme Translations

中文 / English / [fa_IR](README_fa.md)

## 下载 / Download

### GitHub Releases (Portable ZIP)

便携格式，无安装器。转到 Releases 下载预编译的二进制文件，解压后即可使用。

[![GitHub All Releases](https://img.shields.io/github/downloads/Matsuridayo/nekoray/total?label=downloads-total&logo=github&style=flat-square)](https://github.com/Matsuridayo/nekoray/releases)

[下载 / Download](https://github.com/Matsuridayo/nekoray/releases)

[安装包的说明，如果你不知道要下载哪一个](https://github.com/MatsuriDayo/nekoray/wiki/Installation-package-description)

### Package

1. AUR: [nekoray-git](https://aur.archlinux.org/packages/nekoray-git)
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
* Custom Config
* Custom Core

## 订阅 / Subscription

* Raw: some widely used formats (like shadowsocks, clash and v2rayN)
* 原始格式：一些广泛使用的格式（如 shadowsocks、clash 和 v2rayN）

## 运行参数

[运行参数](docs/RunFlags.md)

## Windows 运行

若提示 DLL 缺失，无法运行，请下载 安装 [微软 C++ 运行库](https://aka.ms/vs/17/release/vc_redist.x64.exe)

## Linux 运行

[Linux 运行教程](docs/Run_Linux.md)

## macOS

由于缺乏维护，不再提供 macOS 版本下载。

您还可以在下面的存储库中非正式地获取更新版本。[非官方发布 / Unofficial releases](https://github.com/aaaamirabbas/nekoray-macos/releases)

仍可以参照下方教程自行编译，常见问题请参考历史 Issue，欢迎 Pull Request 修复。

## 编译教程 / Compile Tutorial

请看 [技术文档 / Technical documentation](https://github.com/MatsuriDayo/nekoray/tree/main/docs)

## 捐助 / Donate

Donations are welcome to support project development.

欢迎捐赠以支持项目开发。

USDT TRC20

`TRhnA7SXE5Sap5gSG3ijxRmdYFiD4KRhPs`

XMR

`49bwESYQjoRL3xmvTcjZKHEKaiGywjLYVQJMUv79bXonGiyDCs8AzE3KiGW2ytTybBCpWJUvov8SjZZEGg66a4e59GXa6k5`

## Credits

Core:

- [v2fly/v2ray-core](https://github.com/v2fly/v2ray-core) ( < 3.10 )
- [MatsuriDayo/Matsuri](https://github.com/MatsuriDayo/Matsuri) ( < 3.10 )
- [MatsuriDayo/v2ray-core](https://github.com/MatsuriDayo/v2ray-core) ( < 3.10 )
- [XTLS/Xray-core](https://github.com/XTLS/Xray-core) ( >= 3.10 )
- [MatsuriDayo/Xray-core](https://github.com/MatsuriDayo/Xray-core) ( >= 3.10 )
- [SagerNet/sing-box](https://github.com/SagerNet/sing-box)
- [Matsuridayo/sing-box-extra](https://github.com/MatsuriDayo/sing-box-extra)

Gui:

- [Qv2ray](https://github.com/Qv2ray/Qv2ray)
- [Qt](https://www.qt.io/)
- [protobuf](https://github.com/protocolbuffers/protobuf)
- [yaml-cpp](https://github.com/jbeder/yaml-cpp)
- [zxing-cpp](https://github.com/nu-book/zxing-cpp)
- [QHotkey](https://github.com/Skycoder42/QHotkey)
- [AppImageKit](https://github.com/AppImage/AppImageKit)
