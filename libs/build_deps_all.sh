#!/bin/bash
set -e

cd libs

# libs/deps/...
mkdir -p deps; cd deps
INSTLL_PREFIX=$PWD/built
mkdir -p $INSTLL_PREFIX

#### yaml-cpp ####
curl -L -o dl.zip https://github.com/jbeder/yaml-cpp/archive/refs/tags/yaml-cpp-0.7.0.zip
unzip dl.zip

cd yaml-*
mkdir -p build; cd build

cmake .. -GNinja -DBUILD_TESTING=OFF -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=$INSTLL_PREFIX
ninja && ninja install

cd ../..

#### ZXing ####
curl -L -o dl.zip https://github.com/nu-book/zxing-cpp/archive/refs/tags/v1.3.0.zip
unzip dl.zip

cd zxing-*
mkdir -p build; cd build

cmake .. -GNinja -DCMAKE_BUILD_TYPE=Release -DBUILD_EXAMPLES=OFF -DBUILD_BLACKBOX_TESTS=OFF -DCMAKE_INSTALL_PREFIX=$INSTLL_PREFIX
ninja && ninja install

cd ../..

#### clean ####
rm -rf dl.zip yaml-* zxing-*
