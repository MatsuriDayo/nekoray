#!/bin/bash
set -e

source libs/env_deploy.sh
DEST=$DEPLOYMENT/linux64
rm -rf $DEST
mkdir -p $DEST

#### copy binary ####
cp $BUILD/nekoray $DEST

#### Download: prebuilt runtime ####
curl -Lso usr.zip https://github.com/MatsuriDayo/nekoray_qt_runtime/releases/download/20220503/20230202-5.12.8-ubuntu20.04-linux64.zip
unzip usr.zip
mv usr $DEST


#### copy so ####
# 5.11 looks buggy on new systems...
exit

USR_LIB=/usr/lib/x86_64-linux-gnu
mkdir usr
pushd usr
mkdir lib
pushd lib
cp $USR_LIB/libQt5Core.so.5 .
cp $USR_LIB/libQt5DBus.so.5 .
cp $USR_LIB/libQt5Gui.so.5 .
cp $USR_LIB/libQt5Network.so.5 .
cp $USR_LIB/libQt5Svg.so.5 .
cp $USR_LIB/libQt5Widgets.so.5 .
cp $USR_LIB/libQt5X11Extras.so.5 .
cp $USR_LIB/libQt5XcbQpa.so.5 .
cp $USR_LIB/libdouble-conversion.so.? .
cp $USR_LIB/libxcb-util.so.? .
cp $USR_LIB/libicuuc.so.?? .
cp $USR_LIB/libicui18n.so.?? .
cp $USR_LIB/libicudata.so.?? .
popd
mkdir plugins
pushd plugins
cp -r $USR_LIB/qt5/plugins/bearer .
cp -r $USR_LIB/qt5/plugins/iconengines .
cp -r $USR_LIB/qt5/plugins/imageformats .
cp -r $USR_LIB/qt5/plugins/platforminputcontexts .
cp -r $USR_LIB/qt5/plugins/platforms .
cp -r $USR_LIB/qt5/plugins/xcbglintegrations .
popd
popd
mv usr $DEST
