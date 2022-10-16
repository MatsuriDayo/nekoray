#!/bin/bash
set -e

if [[ $(uname -m) == 'arm64' ]]; then
  ARCH="arm64"
else
  ARCH="amd64"
fi

source libs/deploy_common.sh
DEST=$DEPLOYMENT/macos-$ARCH
rm -rf $DEST
mkdir -p $DEST

#### deploy qt & DLL runtime => dmg ####
pushd $BUILD
macdeployqt nekoray.app -dmg -verbose=3
popd

#### copy dmg ####
cp $BUILD/*.dmg $DEST
