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
COMMIT=$(cat matsuri_commit.txt)
version_standalone="nekoray-"$(cat nekoray_version.txt)

pushd ..
git clone --no-checkout https://github.com/MatsuriDayo/Matsuri.git

pushd Matsuri
git checkout $COMMIT
bash buildScript/lib/core/clone.sh # Get v2ray source of the commit
popd

pushd v2ray-core
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
curl -Lso $DEST/geoip.dat "https://github.com/Loyalsoldier/v2ray-rules-dat/releases/download/202204292212/geoip.dat"
curl -Lso $DEST/geosite.dat "https://github.com/Loyalsoldier/v2ray-rules-dat/releases/download/202204292212/geosite.dat"

#### copy assets ####
cp assets/* $DEST
