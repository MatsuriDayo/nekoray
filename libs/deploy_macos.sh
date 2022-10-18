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

#### copy golang & asset => .app ####
pushd download-artifact
find . -name artifacts.tgz | xargs -n1 tar xvzf
mv deployment/assets/* deployment/macos-$ARCH
mv deployment/macos-$ARCH/* $BUILD/nekoray.app/Contents/MacOS
popd

#### deploy qt & DLL runtime => dmg ####
pushd $BUILD
macdeployqt nekoray.app -dmg -verbose=3
popd

#### copy dmg ####
cp $BUILD/*.dmg $DEST
