#!/bin/bash
set -e

cd libs

# libs/deps/...
mkdir -p deps; cd deps
INSTLL_PREFIX=$PWD/built

#### grpc ####
git clone --recurse-submodules -b v1.33.1 --depth 1 --shallow-submodules https://github.com/grpc/grpc

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
