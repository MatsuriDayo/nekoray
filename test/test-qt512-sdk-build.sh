QT=$PWD/qtsdk/5.12.12/gcc_64
BUILD=build-sdk-qt512
rm -rf $BUILD
mkdir -p $BUILD
cd $BUILD
cmake -GNinja -DCMAKE_PREFIX_PATH=$QT ..
ninja
