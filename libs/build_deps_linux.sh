#!/bin/bash
set -e

cd libs

# libs/deps/...
mkdir -p deps; cd deps
INSTLL_PREFIX=$PWD/built

#### grpc ####
#备注： v1.33.1 是 Windows 上面用的万年版，Ubuntu 18.04 的环境似乎也可以，以后和 Go 部分不兼容了再改
git clone --recurse-submodules -b v1.33.1 --depth 1 --shallow-submodules https://github.com/grpc/grpc

#备注：交叉编译要在 host 也安装 grpc 并且版本一致,编译安装，同参数，安装到 /usr/local

cd grpc
mkdir -p cmake/build
pushd cmake/build
cmake ../.. -GNinja -DgRPC_INSTALL=ON \
      -DgRPC_PROTOBUF_PACKAGE_TYPE=CONFIG \
      -DgRPC_BUILD_TESTS=OFF \
      -DCMAKE_INSTALL_PREFIX=$INSTLL_PREFIX
ninja && ninja install
popd
cd ..

#### clean ####
rm -rf grpc
