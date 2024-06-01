提示：编译通过不代表所有功能可用，Tun 系统代理 这种功能大概率是不可用的。如果您懂得修复，我们欢迎您的 Pull Request。

Tip: Passing the compilation does not mean that all functions are available, and the function of Tun system proxy is probably not available. If you know how to fix it, we welcome your Pull Request.

## git clone 源码

```
git clone https://github.com/MatsuriDayo/nekoray.git --recursive
```

## 先决条件

1. 安装 `CMake` `Ninja` `npm`
2. 安装 Qt `5.15.x` 编译环境，并设置好有关环境变量，以免 CMake 找不到 Qt。 (`qtbase` `qtsvg` `qttools`)
3. 安装 `appdmg` (`sudo npm install -g appdmg`)

## CMake 参数

同 Build_Linux

## C++ 部分

### 编译安装 C/C++ 依赖

（这一步可能要挂梯）

```shell
./libs/build_deps_all.sh
```

### 编译本体

```shell
mkdir build
cd build
cmake -GNinja -DCMAKE_BUILD_TYPE=Release -DNKR_PACKAGE_MACOS=1 ..
ninja
```

### Go 部分编译

请看 [Build_Core.md](./Build_Core.md)

## 打包

如有疑问，请参考[这个脚本](https://github.com/MatsuriDayo/nekoray/blob/main/libs/deploy_macos.sh)

### 准备core

将 编译得到的 `nekoray_core` `nekobox_core` 放入 `build/nekoray.app/Contents/MacOS`

### 准备geo文件（可选）

```shell
curl -Lso build/nekoray.app/Contents/MacOS/geoip.dat "https://github.com/Loyalsoldier/v2ray-rules-dat/releases/latest/download/geoip.dat"
curl -Lso build/nekoray.app/Contents/MacOS/geosite.dat "https://github.com/v2fly/domain-list-community/releases/latest/download/dlc.dat"
curl -Lso build/nekoray.app/Contents/MacOS/geoip.db "https://github.com/SagerNet/sing-geoip/releases/latest/download/geoip.db"
curl -Lso build/nekoray.app/Contents/MacOS/geosite.db "https://github.com/SagerNet/sing-geosite/releases/latest/download/geosite.db"
```

如果打包时没有包含geo文件，可在应用内设置 `v2ray资源路径` 代替。

### 在nekoray目录下打包

```shell
appdmg appdmg.json /path/to/nekoray.dmg
```
