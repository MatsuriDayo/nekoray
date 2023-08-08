#!/bin/bash
set -e

if [[ $(uname -m) == 'arm64' ]]; then
  ARCH="arm64"
else
  ARCH="amd64"
fi

source libs/env_deploy.sh
DEST=$DEPLOYMENT/macos-$ARCH
rm -rf $DEST
mkdir -p $DEST

#### copy golang & public_res => .app ####
pushd download-artifact
find . -name artifacts.tgz | xargs -n1 tar xvzf
mv deployment/public_res/* deployment/macos-$ARCH
mv deployment/macos-$ARCH/* $BUILD/nekoray.app/Contents/MacOS
popd

#### deploy qt & DLL runtime => .app ####
pushd $BUILD
macdeployqt nekoray.app -verbose=3
popd

#### pack dmg ###
sudo npm install -g appdmg
appdmg appdmg.json $DEST/nekoray.dmg
