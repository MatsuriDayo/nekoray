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

COMMIT=$(cat matsuri_commit.txt)
version_standalone="nekoray-"$(cat nekoray_version.txt)

git clone --no-checkout https://github.com/MatsuriDayo/Matsuri.git
git clone https://github.com/MatsuriDayo/v2ray-core.git

pushd Matsuri
git checkout $COMMIT
popd

pushd v2ray-core
version_v2ray=$(git log --pretty=format:'%h' -n 1)
popd

pushd Matsuri/libcore/v2ray
go build -trimpath -ldflags "-w -s -X main.version_v2ray=$version_v2ray -X main.version_standalone=$version_standalone"
rm *.go
mv v2ray* $DEST/nekoray_core
popd

#### copy tanslations ####
# TODO

#### geoip ####
curl -Lso $DEST/geoip.dat "https://github.com/Loyalsoldier/v2ray-rules-dat/releases/download/202204292212/geoip.dat"
curl -Lso $DEST/geosite.dat "https://github.com/Loyalsoldier/v2ray-rules-dat/releases/download/202204292212/geosite.dat"

#### copy assets ####
cp assets/* $DEST
