#!/bin/bash
set -e

source libs/deploy_common.sh

#### updater to launcher ####
mv $DEST/updater $DEST/launcher

#### copy exe ####
cp build/nekoray $DEST

#### prebuilt runtime ####
curl -Lso usr.zip https://github.com/MatsuriDayo/nekoray_qt_runtime/releases/download/20220503/5.15.0-linux64.zip
unzip usr.zip
mv usr $DEST

#### copy runtime ####
LIB=$SRC_ROOT/libs/deps/built/lib
cp $LIB/libZXing.so.1 $DEST/usr/lib

#### pack tar ####
chmod +x $DEST/nekoray $DEST/nekoray_core $DEST/launcher
tar cvzf $SRC_ROOT/deployment/$version_standalone-linux64.tar.gz -C $SRC_ROOT/deployment nekoray
rm -rf $DEST
