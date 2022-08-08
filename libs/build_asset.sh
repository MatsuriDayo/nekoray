#!/bin/bash
set -e

source libs/deploy_common.sh
DEST=$DEPLOYMENT/assets
rm -rf $DEST
mkdir -p $DEST

#### Download: geoip ####
curl -Lso $DEST/geoip.dat "https://github.com/Loyalsoldier/v2ray-rules-dat/releases/download/202206042210/geoip.dat"
curl -Lso $DEST/geosite.dat "https://github.com/v2fly/domain-list-community/releases/download/20220604062951/dlc.dat"

#### copy assets ####
cp assets/* $DEST
