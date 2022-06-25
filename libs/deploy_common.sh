SRC_ROOT="$PWD"
DEST="$PWD/deployment/nekoray"
mkdir -p $DEST

#### build & copy golang part ####
export CGO_ENABLED=0

pushd updater
go build -trimpath -ldflags "-w -s"
rm *.go
mv updater* $DEST
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

#### nekoray_core ####
pushd go
go build -trimpath -ldflags "-w -s -X main.version_v2ray=$version_v2ray -X main.version_standalone=$version_standalone"
rm *.go
mv nekoray_core* $DEST/nekoray_core
popd

#### geoip ####
curl -Lso $DEST/geoip.dat "https://github.com/Loyalsoldier/v2ray-rules-dat/releases/download/202206042210/geoip.dat"
curl -Lso $DEST/geosite.dat "https://github.com/v2fly/domain-list-community/releases/download/20220604062951/dlc.dat"

#### copy assets ####
cp assets/* $DEST
