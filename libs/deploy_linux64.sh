#!/bin/bash
set -e

source libs/deploy_common.sh
DEST=$DEPLOYMENT/linux64
rm -rf $DEST
mkdir -p $DEST

#### copy binary ####
cp $BUILD/nekoray $DEST

#### Download: prebuilt runtime ####
curl -Lso usr.zip https://github.com/MatsuriDayo/nekoray_qt_runtime/releases/download/20220503/20220705-5.15.2-linux64.zip
unzip usr.zip
rm usr.zip
mv usr $DEST
