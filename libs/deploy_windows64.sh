#!/bin/bash
set -e

source libs/deploy_common.sh

#### Go: sing-box ####
pushd $BUILD
curl -Lso sing-box.zip https://github.com/SagerNet/sing-box/archive/44068500bf8842c8849068676a61ecca8f127a33.zip
unzip sing-box.zip
pushd sing-box-*/cmd/sing-box
go build -o $DEST -trimpath -ldflags "-w -s"
popd
popd

#### copy exe ####
cp $BUILD/nekoray.exe $DEST

#### deploy qt & DLL runtime ####
pushd $DEST
windeployqt nekoray.exe --no-compiler-runtime --no-system-d3d-compiler --no-opengl-sw --verbose 2
curl -LSsO https://github.com/MatsuriDayo/nekoray_qt_runtime/releases/download/20220503/libcrypto-1_1-x64.dll
curl -LSsO https://github.com/MatsuriDayo/nekoray_qt_runtime/releases/download/20220503/libssl-1_1-x64.dll
rm -rf translations
popd

#### pack zip ####
7z a $SRC_ROOT/deployment/$version_standalone-windows64.zip $DEST
cp $BUILD/*.pdb $SRC_ROOT/deployment/
rm -rf $DEST $BUILD
