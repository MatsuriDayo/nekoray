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
rm -rf translations
rm -rf libEGL.dll libGLESv2.dll Qt6Pdf.dll
curl -fLSO https://github.com/MatsuriDayo/nekoray_qt_runtime/releases/download/20220503/libcrypto-3-x64.dll
curl -fLSO https://github.com/MatsuriDayo/nekoray_qt_runtime/releases/download/20220503/libssl-3-x64.dll
popd

#### prepare deployment ####
cp $BUILD/*.pdb $DEPLOYMENT
