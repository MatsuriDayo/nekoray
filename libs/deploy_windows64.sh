#!/bin/bash
set -e

source libs/deploy_common.sh

#### fuck you windows ####
mv $DEST/nekoray_core $DEST/nekoray_core.exe

#### copy dll (except qt and msvc) & exe ####
cp build/*.dll build/nekoray.exe $DEST

#### deploy qt & msvc runtime ####
pushd $DEST
windeployqt nekoray.exe --no-compiler-runtime --no-system-d3d-compiler --no-opengl-sw --verbose 2
rm -rf translations
popd

#### pack zip ####
7z a $SRC_ROOT/deployment/$version_standalone-windows64.zip $DEST
cp build/*.pdb $SRC_ROOT/deployment/
rm -rf $DEST build
