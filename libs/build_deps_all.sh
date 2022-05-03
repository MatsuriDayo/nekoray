#!/bin/bash
set -e

cd libs

# libs/deps/...
mkdir -p deps; cd deps
INSTLL_PREFIX=$PWD/built

#### yaml-cpp ####
curl -L -o dl.zip https://github.com/jbeder/yaml-cpp/archive/refs/tags/yaml-cpp-0.7.0.zip
unzip dl.zip

cd yaml-*
mkdir -p build; cd build

cmake .. -GNinja -DBUILD_TESTING=OFF -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=$INSTLL_PREFIX
ninja && ninja install

cd ../..

#### clean ####
rm -rf dl.zip yaml-*
