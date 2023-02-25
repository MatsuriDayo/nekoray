Compile Nekoray for Linux

## git clone source code

```
git clone https://github.com/MatsuriDayo/nekoray.git --recursive
```

## simple compilation method

conditions：

1. C++ dependencies：`protobuf yaml-cpp zxing-cpp` It has been installed with the package manager and meets the version requirements.
2. Installed `qtbase` `qtsvg` `qttools` `qtx11extras`
3. Qt installed `5.12.x` or `5.15.x`
4. system is `x86-64-linux-gnu`

```shell
mkdir build
cd build
cmake -GNinja ..
ninja
```

After compiling, get `nekoray`

Unzip the compressed package of Release and replace the `nekoray`，delete `launcher` Ready to use.

## complex compilation method

### CMake parameters

| CMake parameters  |      Defaults     |                         meaning                          |
|-------------------|-------------------|-----------------------------------------------------------|
| QT_VERSION_MAJOR  | 5                 | QT version                                                |
| NKR_NO_EXTERNAL   |                   | Does not contain external C/C++ dependencies (all below)  |
| NKR_NO_YAML       |                   | does not contain yaml-cpp                                 |
| NKR_NO_QHOTKEY    |                   | does not contain qhotkey                                  |
| NKR_NO_ZXING      |                   | does not contain zxing                                    |
| NKR_NO_GRPC       |                   | does not contain gRPC                                     |
| NKR_NO_QUICKJS    |                   | does not contain quickjs                                  |
| NKR_PACKAGE       |                   | Compile package version (aur)                             |
| NKR_PACKAGE_MACOS |                   | Compile macos version                                     |
| NKR_LIBS          | ./libs/deps/built | Dependency search directory                               |
| NKR_DISABLE_LIBS  |                   | disable NKR_LIBS                                          |

1. `NKR_LIBS` The value of will be appended to the `CMAKE_PREFIX_PATH`
2. `NKR_PACKAGE` After opening, `NKR_LIBS` The default value for `./libs/deps/package` ，see specific dependencies `build_deps_all.sh`
3. `NKR_PACKAGE_MACOS` or `NKR_PACKAGE` After opening, the application will use the appdata directory to store configuration, and functions such as automatic update will be disabled.  

### C++ part

When your distribution does not have the above C++ dependent packages, or the version does not meet the requirements, you can refer to `build_deps_all.sh` The compile script compiles itself.  

Condition: Qt installed `5.12.x` or `5.15.x`

#### Compile and install C/C++ dependencies

(This step may require a ladder)

```shell
./libs/build_deps_all.sh
```

#### compile ontology

```shell
mkdir build
cd build
cmake -GNinja ..
ninja
```

After compiling, get `nekoray`

### Go part

1. Bundle `Matsuridayo/v2ray-core` placed in `../`
2. Enter `go/cmd/nekoray_core` folder `go build` get `nekoray_core`.
3. Enter `go/cmd/nekobox_core` folder `go build` get `nekobox_core`.

Please refer to the specific compilation process  `libs/get_source.sh` `libs/build_go.sh`

Unofficial builds do not require compilation `updater` `launcher`
