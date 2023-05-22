#!/bin/bash
set -e

source libs/deploy_common.sh
if [ "$DL_QT_VER" == "5.15" ]; then
  DEST=$DEPLOYMENT/windows7-x64
else
  DEST=$DEPLOYMENT/windows64
fi
rm -rf $DEST
mkdir -p $DEST

#### copy exe ####
cp $BUILD/nekoray.exe $DEST

#### deploy qt & DLL runtime ####
pushd $DEST
windeployqt nekoray.exe --no-compiler-runtime --no-system-d3d-compiler --no-opengl-sw --verbose 2
rm -rf translations
rm -rf libEGL.dll libGLESv2.dll Qt6Pdf.dll

if [ "$DL_QT_VER" != "5.15" ]; then
  curl -fLSO https://github.com/MatsuriDayo/nekoray_qt_runtime/releases/download/20220503/libcrypto-3-x64.dll
  curl -fLSO https://github.com/MatsuriDayo/nekoray_qt_runtime/releases/download/20220503/libssl-3-x64.dll
fi

popd

#### prepare deployment ####
cp $BUILD/*.pdb $DEPLOYMENT
