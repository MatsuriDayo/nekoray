#!/bin/bash

cd libs

mkdir -p deps; cd ./deps
mkdir -p downloaded; cd ./downloaded;

NAME=$1
echo "Downloading: $NAME"
curl -sL $2 -o $NAME;

cd ..

for f in $(ls ./downloaded)
do
    7z x -y ./downloaded/$f
done

# libs/deps/windows-x64/installed/

rm -rf downloaded
