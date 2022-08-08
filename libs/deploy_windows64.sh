#!/bin/bash
set -e

source libs/deploy_common.sh
DEST=$DEPLOYMENT/windows64
rm -rf $DEST
mkdir -p $DEST

#### copy exe ####
cp $BUILD/nekoray.exe $DEST

#### deploy qt & DLL runtime ####
pushd $DEST
windeployqt nekoray.exe --no-compiler-runtime --no-system-d3d-compiler --no-opengl-sw --verbose 2
curl -LSsO https://github.com/MatsuriDayo/nekoray_qt_runtime/releases/download/20220503/libcrypto-1_1-x64.dll
curl -LSsO https://github.com/MatsuriDayo/nekoray_qt_runtime/releases/download/20220503/libssl-1_1-x64.dll
rm -rf translations
popd

#### prepare deployment ####
cp $BUILD/*.pdb $DEPLOYMENT
