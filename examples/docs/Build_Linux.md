在 Linux 下编译 Nekoray

### git clone 源码

```
git clone https://github.com/MatsuriDayo/nekoray.git --recursive
```

### 简单编译法

条件：

1. C++ 依赖：`protobuf yaml-cpp zxing-cpp` 已用包管理器安装，并符合版本要求
2. 已安装 `qtbase` `qtsvg` `qttools` `qtx11extras`
3. Qt 版本必须大于等于 5.15
4. 系统为 `x86-64-linux-gnu`

```shell
mkdir build
cd build
cmake -GNinja ..
ninja
```

编译完成后得到 `nekoray`

解压 Release 的压缩包，替换其中的 `nekoray`，删除 `launcher` 即可使用。

### 复杂编译法

#### CMake 参数

| CMake 参数         | 默认值 | 含义                      |
|------------------|-----|-------------------------|
| QT_VERSION_MAJOR | 5   | QT版本                    |
| NKR_NO_EXTERNAL  |     | 不包含外部C++依赖(以下所有) |
| NKR_NO_YAML     |     | 不包含yaml-cpp                 |
| NKR_NO_QHOTKEY   |     | 不包含qhotkey                 |
| NKR_NO_ZXING     |     | 不包含zxing                 |
| NKR_NO_GRPC      |     | 不包含gRPC                 |

#### C++ 部分

当您的发行版没有上面几个 C++ 依赖包，或者版本不符合要求时，可以参考 libs 文件夹内的默认编译脚本自行编译。

依赖搜寻 prefix 为 `libs/deps/bulit`

条件：

1. Qt 版本必须大于等于 5.15

首先编译安装C++依赖（这一步可能要挂梯）

```shell
./libs/build_deps_all.sh
```

然后编译本体

```shell
mkdir build
cd build
cmake -GNinja ..
ninja
```

编译完成后得到 `nekoray`

#### Go 部分

1. 把 `Matsuridayo/Matsuri` `Matsuridayo/v2ray-core` 置于 `../`
2. 进入 `go/cmd/nekoray_core` 文件夹 `go build` 得到 `nekoray_core`。
3. 进入 `go/cmd/nekobox_core` 文件夹 `go build` 得到 `nekobox_core`。

非官方构建无需编译 `updater` `launcher`
