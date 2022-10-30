# NekoRay

Qt based cross-platform GUI proxy configuration manager (backend: v2ray / sing-box)

Support Windows / Linux / macOS amd64 out of the box now.

基于 Qt 的跨平台代理配置管理器 (后端 v2ray / sing-box)

目前支持 Windows / Linux / macOS amd64 开箱即用

## 下载 Download

便携格式，无安装器。转到 Releases 下载预编译的二进制文件，解压后即可使用。

### GitHub Releases

[![GitHub All Releases](https://img.shields.io/github/downloads/Matsuridayo/nekoray/total?label=downloads-total&logo=github&style=flat-square)](https://github.com/Matsuridayo/nekoray/releases) [Download](https://github.com/Matsuridayo/nekoray/releases)

## 更改记录 & 发布频道 Changelog & Telegram channel

https://t.me/Matsuridayo

## 项目主页 & 文档 Homepage & Documents

https://matsuridayo.github.io

## 运行参数

- `-many` 无视同目录正在运行的实例，强行开启新的实例 (0.11+)
- `-appdata` 开启后配置文件会放在共享目录，无法多开和自动升级 (0.11+)

## Windows 运行

若提示 DLL 缺失，无法运行，请下载 安装 [微软 C++ 运行库](https://aka.ms/vs/17/release/vc_redist.x64.exe)

## Linux 运行

**使用 Linux 系统相信您已具备基本的排错能力，
本项目不提供特定发行版/架构的支持，预编译文件不能满足您的需求时，请自行编译/适配。**

要求：已安装主流的发行版和 xcb 桌面环境。

运行： `./launcher` 或 部分系统可双击打开

launcher 参数

* `./launcher -- -appdata` ( `--` 后的参数传递给主程序 )
* `-debug` Debug mode
* `-theme` Use local QT theme (unstable) (1.0+)

已知部分 Linux 发行版无法使用预编译版、非 x86_64 暂无适配，可以尝试自行编译。

Ubuntu 22.04: `sudo apt install libxcb-xinerama0`

If you are using Arch Linux, you can install via [AUR](https://aur.archlinux.org/packages/nekoray)

## 编译教程

[Linux](examples/docs/Build_Linux.md)

[Windows](examples/docs/Build_Windows.md)

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
