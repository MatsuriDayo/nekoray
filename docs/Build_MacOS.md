在 macOS 下编译 Nekoray

## git clone 源码

```
git clone https://github.com/MatsuriDayo/nekoray.git --recursive
```

## 全部依赖:
1. C++ 依赖：`protobuf yaml-cpp zxing-cpp` 已用包管理器安装，并符合版本要求。
2. 已安装 `qtbase` `qtsvg` `qttools` `qtx11extras`。
3. 已安装 Qt `5.12.x` 或 `5.15.x`，并设置好有关环境变量。
4. 安装`appdmg`(可以来自包管理器`npm`)


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

当您的没有上面几个 C++ 依赖包，或者版本不符合要求时，可以参考 `build_deps_all.sh` 编译脚本自行编译。

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
cmake -GNinja -DCMAKE_BUILD_TYPE=Release -DNKR_PACKAGE_MACOS=1 ..
ninja
```

编译完成后得到 `nekoray`

### Go 部分

1. 把 `Matsuridayo/Matsuri` `Matsuridayo/v2ray-core` 置于 `../`
2. 进入 `go/cmd/nekoray_core` 文件夹 `go build` 得到 `nekoray_core`。
3. 进入 `go/cmd/nekobox_core` 文件夹 `go build` 得到 `nekobox_core`。

非官方构建无需编译 `updater` `launcher`

### 打包

安装`appdmg`
```shell
sudo npm install -g appdmg
```

然后在nekoray目录下打包
```shell
appdmg appdmg.json /path/to/nekoray.dmg
```

### 使用

右键nekoray，选择"显示包内容"，将core和geo文件放在`Contents/MacOS/`中。

#### geo*
```shell
curl -Lso /Applications/nekoray.app/Contents/MacOS/geoip.dat "https://github.com/Loyalsoldier/v2ray-rules-dat/releases/latest/download/geoip.dat"
curl -Lso /Applications/nekoray.app/Contents/MacOS/geosite.dat "https://github.com/v2fly/domain-list-community/releases/latest/download/dlc.dat"
curl -Lso /Applications/nekoray.app/Contents/MacOS/geoip.db "https://github.com/SagerNet/sing-geoip/releases/latest/download/geoip.db"
curl -Lso /Applications/nekoray.app/Contents/MacOS/geosite.db "https://github.com/SagerNet/sing-geosite/releases/latest/download/geosite.db"
```
