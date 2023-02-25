Compile Nekoray for macOS

## git clone source code

```
git clone https://github.com/MatsuriDayo/nekoray.git --recursive
```

## prerequisites

1. Install `CMake` `Ninja` `npm`
2. Install Qt `5.15.x` Compile the environment, and set the relevant environment variables so that CMake cannot find Qt. (`qtbase` `qtsvg` `qttools`)
3. Install `appdmg` (`sudo npm install -g appdmg`)

## CMake parameters

same Build_Linux

## C++ part

### Compile and install C/C++ dependencies

(This step may require a ladder)

```shell
./libs/build_deps_all.sh
```

### compile ontology

```shell
mkdir build
cd build
cmake -GNinja -DCMAKE_BUILD_TYPE=Release -DNKR_PACKAGE_MACOS=1 ..
ninja
```

## Go part

same Build_Linux

## Pack

In case of doubt, refer to [this script](https://github.com/MatsuriDayo/nekoray/blob/main/libs/deploy_macos.sh)

### prepare core

will compile the resulting `nekoray_core` `nekobox_core` put into a `build/nekoray.app/Contents/MacOS`

### Prepare geo file (optional)

```shell
curl -Lso build/nekoray.app/Contents/MacOS/geoip.dat "https://github.com/Loyalsoldier/v2ray-rules-dat/releases/latest/download/geoip.dat"
curl -Lso build/nekoray.app/Contents/MacOS/geosite.dat "https://github.com/v2fly/domain-list-community/releases/latest/download/dlc.dat"
curl -Lso build/nekoray.app/Contents/MacOS/geoip.db "https://github.com/SagerNet/sing-geoip/releases/latest/download/geoip.db"
curl -Lso build/nekoray.app/Contents/MacOS/geosite.db "https://github.com/SagerNet/sing-geosite/releases/latest/download/geosite.db"
```

If the geo file is not included in the package, it can be set in the application `v2ray resource path` replace.

### Packaged under the nekoray directory

```shell
appdmg appdmg.json /path/to/nekoray.dmg
```
