#!/bin/bash
set -e

source libs/deploy_common.sh

#### get source ####
COMMIT_M=$(cat matsuri_commit.txt)
COMMIT_V=$(cat core_commit.txt)

pushd ..

git clone --no-checkout https://github.com/MatsuriDayo/Matsuri.git
git clone --no-checkout https://github.com/MatsuriDayo/v2ray-core.git

pushd Matsuri
git checkout $COMMIT_M
popd

pushd v2ray-core
git checkout $COMMIT_V
popd

popd
