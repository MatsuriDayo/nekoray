#!/bin/bash
set -e

source libs/deploy_common.sh

#### fuck you windows ####
mv $DEST/nekoray_core $DEST/nekoray_core.exe

#### copy dll (except qt and msvc) & exe ####
cp build/*.dll build/nekoray.exe $DEST

#### deploy qt & msvc runtime ####
cd $DEST
windeployqt nekoray.exe --compiler-runtime --verbose 2
rm -rf vc_redist.*.exe

#### pack zip ####
7z a $SRC_ROOT/deployment/$version_standalone-windows64.zip $DEST
rm -rf $DEST
