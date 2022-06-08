rm -rf build-qt6
mkdir -p build-qt6
cd build-qt6
cmake -GNinja -DQT_VERSION_MAJOR=6 ..
ninja
