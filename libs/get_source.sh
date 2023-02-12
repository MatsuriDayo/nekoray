#!/bin/bash
set -e

source libs/deploy_common.sh

#### get source ####
COMMIT_V=$(cat core_commit.txt)

pushd ..

git clone --no-checkout https://github.com/MatsuriDayo/v2ray-core.git

pushd v2ray-core
git checkout $COMMIT_V
popd

popd
