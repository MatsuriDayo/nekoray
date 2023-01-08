# NekoRay

Qt based cross-platform GUI proxy configuration manager (backend: v2ray / sing-box)

Support Windows / Linux / macOS amd64 out of the box now.

Cross-platform proxy configuration manager based on Qt (backend v2ray / sing-box).

Currently supports Windows / Linux / macOS on AMD64 architecture, ready to use out of the box.

## Download

### GitHub Releases (Portable ZIP)

Portable format, no installer. Go to Releases to download the pre-compiled binary file, unzip and use.

[![GitHub All Releases](https://img.shields.io/github/downloads/Matsuridayo/nekoray/total?label=downloads-total&logo=github&style=flat-square)](https://github.com/Matsuridayo/nekoray/releases)

[Download](https://github.com/Matsuridayo/nekoray/releases)

### Package

1. AUR [nekoray-git](https://aur.archlinux.org/packages/nekoray-git)
2. Scoop extras: `scoop install nekoray`

## Changelog & Telegram channel

https://t.me/Matsuridayo

## Homepage & Documents

https://matsuridayo.github.io

## Proxy

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

## Subscription

* Raw: some widely used formats (like shadowsocks, clash and v2rayN)
* Original format: Some widely used formats (such as shadowsocks, clash, and v2rayN).

## Running parameters

- `-many` Ignore instances running in the same directory and forcibly start a new instance (0.11+)
- `-appdata` After enabling, the configuration file will be placed in the shared directory,
  unable to open multiple instances and automatically upgrade. (0.11+)

## Windows 运行
If you are prompted with a missing DLL and are unable to run, please download and install
the [Microsoft C++ Redistributables](https://aka.ms/vs/17/release/vc_redist.x64.exe).

## Linux 运行

[Linux 运行教程](docs/Run_Linux.md)

## Compile Tutorial

[Linux](docs/Build_Linux.md)

[Mac](docs/Build_MacOS.md)

[Windows](docs/Build_Windows.md)

## Donate

Donations are welcome to support project development.

Welcome to donate to support project development.

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
