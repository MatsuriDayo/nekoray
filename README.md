# NekoRay

基于 Qt/C++ 的跨平台代理配置管理器（ 使用 Matsuri 定制版 v2ray-core ）

目前支持 Windows / Linux amd64 开箱即用

Qt/C++ based cross-platform proxy configuration manager ( Use Matsuri custom version of v2ray-core )

Support Windows / Linux amd64 out of the box now.

## 下载 Download

便携格式，无安装器。转到 Releases 下载预编译的二进制文件，解压后即可使用。

### GitHub Releases 下载

[![GitHub All Releases](https://img.shields.io/github/downloads/Matsuridayo/nekoray/total?label=downloads-total&logo=github&style=flat-square)](https://github.com/Matsuridayo/nekoray/releases)

## 更改记录 & 发布频道 Changelog & Telegram channel

https://t.me/Matsuridayo

## 项目主页 & 文档 Homepage & Documents

https://matsuridayo.github.io

### 运行参数

- (0.11+) -many 无视同目录正在运行的实例，强行开启新的实例
- (0.11+) -appdata 开启后配置文件会放在共享目录，无法多开和自动升级

### 代理

| 协议           | 状态     | 配置编辑 | 分享链接生成 | 分享链接解析    | Clash 配置解析 |
|--------------|--------|------|--------|-----------|------------|
| Socks        | ✅      | ✅    | ✅      | ✅         | ✅          | 
| HTTP         | ✅      | ✅    | ✅      | ✅         | ✅          | 
| Shadowsocks  | ✅ (经典) | ✅    | ✅      | 常见格式      | ✅          |
| VMess        | ✅      | ✅    | ✅      | v2rayN 格式 | ✅          |
| Trojan       | ✅      | ✅    | ✅      | 标准&常见格式   | ✅          |
| VLESS        | ✅      | ✅    | ✅      | ✅         | 不适用        |
| NaïveProxy   | ✅      | ✅    | ✅      | ✅         | 不适用        |
| Hysteria     | ✅      | ❌    | ❌      | ❌         | 不适用        |

## Linux 运行 & 简易编译教程

**使用 Linux 系统相信您已具备基本的排错能力，本项目不提供特定发行版/架构的支持，预编译文件不能满足您的需求时，请自行编译/适配。**

系统要求： Qt5 运行环境，一般桌面 Linux 已经安装，如果没有请用包管理器安装，如

`apt install libqt5gui5 libqt5x11extras5`

运行： `./launcher`

已知部分 x86_64 Linux 发行版无法使用预编译版、非 x86_64 暂无适配，可以尝试自行编译。

### 编译

准备工作

```
git submodule init
git submodule update
```

| CMake 参数                | 默认值 | 含义                      |
|-------------------------|-----|-------------------------|
| QT_VERSION_MAJOR        | 5   | QT版本                    |
| NKR_NO_EXTERNAL         |     | 不包含外部C++依赖(如ZXing/gRPC) |
| NKR_NO_GRPC             |     | 不包含gRPC                 |
| NKR_CROSS               |     |                         |

### 简单编译法

条件：

1. C++ 依赖： `qt5 protobuf yaml-cpp zxing-cpp` 已用包管理器安装，并符合版本要求
2. Qt 版本必须大于等于 5.15
3. 系统为 `x86-64-linux-gnu`

```
mkdir build
cd build
cmake -GNinja ..
ninja
```

编译完成后得到 `nekoray`

解压 Release 的压缩包，替换其中的 `nekoray`，删除 `launcher` 即可使用。

### 复杂编译法

C++ 部分

当您的发行版没有上面几个 C++ 依赖包，或者版本不符合要求时，可以参考 libs 文件夹内的默认编译脚本自行编译。

依赖搜寻 prefix 为 `libs/deps/bulit`

编译完成后得到 `nekoray`

Go 部分

1. 把 `Matsuridayo/Matsuri` `Matsuridayo/v2ray-core` 置于 `../`
2. 进入 `go` 文件夹 `go build` 得到 `nekoray_core`。

非官方构建无需编译 `updater` `launcher`

## Credits

Nekoray 的诞生离不开 v2ray 开源项目以及其他开源软件。

依赖:

- [v2fly/v2ray-core](https://github.com/v2fly/v2ray-core)
- [MatsuriDayo/Matsuri](https://github.com/MatsuriDayo/Matsuri)
- [MatsuriDayo/v2ray-core](https://github.com/MatsuriDayo/v2ray-core)
- [Qt](https://www.qt.io/)
- [protobuf](https://github.com/protocolbuffers/protobuf)
- [yaml-cpp](https://github.com/jbeder/yaml-cpp)
- [zxing-cpp](https://github.com/nu-book/zxing-cpp)
- [QHotkey](https://github.com/Skycoder42/QHotkey)

参考：

- [Qv2ray/Qv2ray](https://github.com/Qv2ray/Qv2ray)
- [2dust/v2rayN](https://github.com/2dust/v2rayN)
- [SagerNet/SagerNet](https://github.com/SagerNet/SagerNet)
