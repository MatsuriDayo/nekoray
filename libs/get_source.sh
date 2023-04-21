#!/bin/bash
set -e

source libs/deploy_common.sh
ENV_NEKORAY=1
source libs/get_source_env.sh
pushd ..

####
if [ ! -d "sing-box-extra" ]; then
  git clone --no-checkout https://github.com/MatsuriDayo/sing-box-extra.git
fi
pushd sing-box-extra
git checkout "$COMMIT_SING_BOX_EXTRA"

ENV_SING_BOX_EXTRA=1
source $SRC_ROOT/libs/get_source_env.sh
NO_ENV=1 ./libs/get_source.sh

popd

####
if [ ! -d "v2ray-core" ]; then
  git clone --no-checkout https://github.com/MatsuriDayo/v2ray-core.git
fi
pushd v2ray-core
git checkout "$COMMIT_MATSURI_V2RAY"
popd

popd
