SRC_ROOT="$PWD"
DEST="$PWD/deployment/nekoray"
BUILD="$SRC_ROOT/build"

mkdir -p $DEST
mkdir -p $BUILD

export CGO_ENABLED=0

#### Go: updater ####
pushd updater
go build -o $DEST -trimpath -ldflags "-w -s"
popd

#### libcore ####
COMMIT_M=$(cat matsuri_commit.txt)
COMMIT_V=$(cat core_commit.txt)
version_standalone="nekoray-"$(cat nekoray_version.txt)

pushd ..

git clone --no-checkout https://github.com/MatsuriDayo/Matsuri.git
git clone --no-checkout https://github.com/MatsuriDayo/v2ray-core.git

pushd Matsuri
git checkout $COMMIT_M
popd

pushd v2ray-core
git checkout $COMMIT_V
version_v2ray=$(git log --pretty=format:'%h' -n 1)
popd

popd

#### Go: nekoray_core ####
pushd go
go build -o $DEST -trimpath -ldflags "-w -s -X main.version_v2ray=$version_v2ray -X main.version_standalone=$version_standalone"
popd

#### Download: geoip ####
curl -Lso $DEST/geoip.dat "https://github.com/Loyalsoldier/v2ray-rules-dat/releases/download/202206042210/geoip.dat"
curl -Lso $DEST/geosite.dat "https://github.com/v2fly/domain-list-community/releases/download/20220604062951/dlc.dat"

#### copy assets ####
cp assets/* $DEST
