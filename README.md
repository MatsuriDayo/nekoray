# NekoRay / NekoBox For PC

Qt based cross-platform GUI proxy configuration manager (backend: v2ray / sing-box)

Support Windows / Linux out of the box now.

基于 Qt 的跨平台代理配置管理器 (后端 v2ray / sing-box)

目前支持 Windows / Linux 开箱即用

## Readme Translations

中文 / English / [fa_IR](README_fa.md) / [日本語](README_ja.md)

## 下载 / Download

### GitHub Releases (Portable ZIP)

便携格式，无安装器。转到 Releases 下载预编译的二进制文件，解压后即可使用。

[![GitHub All Releases](https://img.shields.io/github/downloads/Matsuridayo/nekoray/total?label=downloads-total&logo=github&style=flat-square)](https://github.com/Matsuridayo/nekoray/releases)

[下载 / Download](https://github.com/Matsuridayo/nekoray/releases)

[安装包的说明，如果你不知道要下载哪一个](https://github.com/MatsuriDayo/nekoray/wiki/Installation-package-description)

### Package

#### AUR

- [nekoray](https://aur.archlinux.org/packages/nekoray)
- [nekoray-git](https://aur.archlinux.org/packages/nekoray-git) [![AUR CI](https://github.com/MatsuriDayo/nekoray/actions/workflows/update-pkgbuild.yml/badge.svg)](https://github.com/MatsuriDayo/nekoray/actions/workflows/update-pkgbuild.yaml)

#### archlinuxcn

- [nekoray](https://github.com/archlinuxcn/repo/tree/master/archlinuxcn/nekoray)
- [nekoray-git](https://github.com/archlinuxcn/repo/tree/master/archlinuxcn/nekoray-git)

#### Scoop Extras

`scoop install nekoray`

## 更改记录 & 发布频道 / Changelog & Telegram Channel

https://t.me/Matsuridayo

## 项目主页 & 文档 / Homepage & Documents

https://matsuridayo.github.io

## 代理 / Proxy

- SOCKS (4/4a/5)
- HTTP(S)
- Shadowsocks
- VMess
- VLESS
- Trojan
- TUIC ( sing-box )
- NaïveProxy ( Custom Core )
- Hysteria ( Custom Core or sing-box )
- Hysteria2 ( Custom Core or sing-box )
- Custom Outbound
- Custom Config
- Custom Core

## 订阅 / Subscription

- Raw: some widely used formats (like Shadowsocks, Clash and v2rayN)
- 原始格式: 一些广泛使用的格式 (如 Shadowsocks、Clash 和 v2rayN)

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

如果这个项目对您有帮助，可以通过捐赠的方式帮助我们维持这个项目。

捐赠满等额 50 USD 可以在「[捐赠榜](https://mtrdnt.pages.dev/donation_list)」显示头像，如果您未被添加到这里，欢迎联系我们补充。

Donations of 50 USD or more can display your avatar on the [Donation List](https://mtrdnt.pages.dev/donation_list). If you are not added here, please contact us to add it.

USDT TRC20

`TRhnA7SXE5Sap5gSG3ijxRmdYFiD4KRhPs`

XMR

`49bwESYQjoRL3xmvTcjZKHEKaiGywjLYVQJMUv79bXonGiyDCs8AzE3KiGW2ytTybBCpWJUvov8SjZZEGg66a4e59GXa6k5`

- 您也可以通过 [Google Play](https://play.google.com/store/apps/details?id=moe.nb4a) 购买捐赠，另有支付宝微信购买方式，见 TG 频道置顶。
- Donating via [Google Play](https://play.google.com/store/apps/details?id=moe.nb4a), using credit cards.

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
