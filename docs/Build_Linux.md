在 Linux 下编译 Nekoray

## git clone 源码

```
git clone https://github.com/MatsuriDayo/nekoray.git --recursive
```

## 简单编译法

条件：

1. C++ 依赖：`protobuf yaml-cpp zxing-cpp` 已用包管理器安装，并符合版本要求。
2. 已安装 `qtbase` `qtsvg` `qttools` `qtx11extras`
3. 已安装 Qt `5.12.x` 或 `5.15.x`
4. 系统为 `x86-64-linux-gnu`

```shell
mkdir build
cd build
cmake -GNinja ..
ninja
```

编译完成后得到 `nekoray`

解压 Release 的压缩包，替换其中的 `nekoray`，删除 `launcher` 即可使用。

## 复杂编译法

### CMake 参数

| CMake 参数          | 默认值               | 含义                    |
|-------------------|-------------------|-----------------------|
| QT_VERSION_MAJOR  | 5                 | QT版本                  |
| NKR_NO_EXTERNAL   |                   | 不包含外部 C/C++ 依赖 (以下所有) |
| NKR_NO_YAML       |                   | 不包含 yaml-cpp          |
| NKR_NO_QHOTKEY    |                   | 不包含 qhotkey           |
| NKR_NO_ZXING      |                   | 不包含 zxing             |
| NKR_NO_GRPC       |                   | 不包含 gRPC              |
| NKR_NO_QUICKJS    |                   | 不包含 quickjs           |
| NKR_PACKAGE       |                   | 编译 package 版本 (aur)   |
| NKR_PACKAGE_MACOS |                   | 编译 macos 版本           |
| NKR_LIBS          | ./libs/deps/built | 依赖搜索目录                |
| NKR_DISABLE_LIBS  |                   | 禁用 NKR_LIBS           |

1. `NKR_LIBS` 的值会被追加到 `CMAKE_PREFIX_PATH`
2. `NKR_PACKAGE` 打开后，`NKR_LIBS` 的默认值为 `./libs/deps/package` ，具体依赖请看 `build_deps_all.sh`
3. `NKR_PACKAGE_MACOS` 或 `NKR_PACKAGE` 打开后，应用将使用 appdata 目录存放配置，自动更新等功能将被禁用。

### C++ 部分

当您的发行版没有上面几个 C++ 依赖包，或者版本不符合要求时，可以参考 `build_deps_all.sh` 编译脚本自行编译。

条件： 已安装 Qt `5.12.x` 或 `5.15.x`

#### 编译安装 C/C++ 依赖

（这一步可能要挂梯）

```shell
./libs/build_deps_all.sh
```

#### 编译本体

```shell
mkdir build
cd build
cmake -GNinja ..
ninja
```

编译完成后得到 `nekoray`

### Go 部分编译

请看 [Build_Core.md](./Build_Core.md)
