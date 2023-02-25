# NekoRay

Qt based cross-platform GUI proxy configuration manager (backend: v2ray / sing-box)

Support Windows / Linux / ~~macOS amd64~~ out of the box now.

Qt-based cross-platform proxy configuration manager (backend v2ray / sing-box)

Currently supports Windows / Linux / macOS amd64 out of the box

## Readme Translations

[中文](README.md) / English / [fa_IR](README_fa.md)

## Download

### GitHub Releases (Portable ZIP)

for Portable format. Go to the Releases page to download the precompiled binaries, unpack and use them.

[![GitHub All Releases](https://img.shields.io/github/downloads/Matsuridayo/nekoray/total?label=downloads-total&logo=github&style=flat-square)](https://github.com/Matsuridayo/nekoray/releases)

[Download](https://github.com/Matsuridayo/nekoray/releases)

### Package

1. AUR [nekoray-git](https://aur.archlinux.org/packages/nekoray-git)
2. Scoop extras: `scoop install nekoray`

## Changelog & Telegram channel

<https://t.me/Matsuridayo>

## Homepage & Documents

<https://matsuridayo.github.io>

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
* Raw formats: some widely used formats (such as shadowsocks, clash and v2rayN)

## Operating parameters

* `-many` Ignore the running instance of the same directory, forcibly start a new instance (0.11+)
* `-appdata` After opening, the configuration file will be placed in the shared directory, and it cannot be opened more and automatically upgraded (0.11+)

## Windows runs

If it prompts that the DLL is missing and cannot run, please download and install the Microsoft [C++ runtime library](https://aka.ms/vs/17/release/vc_redist.x64.exe)

## Linux runs

[Linux running tutorial](docs/en/Run_Linux.md)

## macOS

Due to lack of maintenance, the macOS version is no longer available for download. The last versions that included dmg were [2.12](https://github.com/MatsuriDayo/nekoray/releases/download/2.12/nekoray-2.12-2023-01-29-macos-amd64.dmg) and [2.11](https://github.com/MatsuriDayo/nekoray/releases/download/2.11/nekoray-2.11-2023-01-13-macos-amd64.dmg)

You can still compile it by yourself by referring to the tutorial below. For common problems, please refer to historical Issues. Pull Requests are welcome to fix.

## Compile Tutorial

[Linux](docs/en/Build_Linux.md)

[Mac](docs/en/Build_MacOS.md)

[Windows](docs/en/Build_Windows.md)

## Donate

Donations are welcome to support project development.

USDT TRC20

`TRhnA7SXE5Sap5gSG3ijxRmdYFiD4KRhPs`

XMR

`49bwESYQjoRL3xmvTcjZKHEKaiGywjLYVQJMUv79bXonGiyDCs8AzE3KiGW2ytTybBCpWJUvov8SjZZEGg66a4e59GXa6k5`

## Credits

* [v2fly/v2ray-core](https://github.com/v2fly/v2ray-core)
* [MatsuriDayo/Matsuri](https://github.com/MatsuriDayo/Matsuri)
* [MatsuriDayo/v2ray-core](https://github.com/MatsuriDayo/v2ray-core)
* [SagerNet/sing-box](https://github.com/SagerNet/sing-box)
* [Qv2ray](https://github.com/Qv2ray/Qv2ray)
* [Qt](https://www.qt.io/)
* [protobuf](https://github.com/protocolbuffers/protobuf)
* [yaml-cpp](https://github.com/jbeder/yaml-cpp)
* [zxing-cpp](https://github.com/nu-book/zxing-cpp)
* [QHotkey](https://github.com/Skycoder42/QHotkey)
