#!/bin/bash

wget --quiet $LLVM_SRC_URL
mkdir -p llvm
tar --strip-components=1 -xf $LLVM_SRC_TAR -C llvm

wget --quiet $LLVM_BIN_URL
tar --strip-components=2 -xf $LLVM_BIN_TAR $LLVM_BIN_TAR_SUBDIR/bin/llvm-tblgen


sudo add-apt-repository -y ppa:ubuntu-toolchain-r/test
sudo apt-get update -qq

sudo apt-get install -qq g++-7
sudo update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-7 90

sudo apt-get install -qq build-essential xorg-dev libgl1-mesa-dev libglew-dev

CMAKE_VERSION=3.3.2
CMAKE_VERSION_DIR=v3.3

CMAKE_OS=Linux-x86_64
CMAKE_TAR=cmake-$CMAKE_VERSION-$CMAKE_OS.tar.gz
CMAKE_URL=http://www.cmake.org/files/$CMAKE_VERSION_DIR/$CMAKE_TAR
CMAKE_DIR=$(pwd)/cmake-$CMAKE_VERSION

wget --quiet $CMAKE_URL
mkdir -p $CMAKE_DIR
tar --strip-components=1 -xzf $CMAKE_TAR -C $CMAKE_DIR
export PATH=$CMAKE_DIR/bin:$PATH

#install git
sudo add-apt-repository -y ppa:git-core/ppa
sudo apt-get update
sudo apt-get install --only-upgrade -y git

CATCH2_REPO_URL=https://github.com/catchorg/Catch2.git

git clone $CATCH2_REPO_URL
cd Catch2
cmake -Bbuild -H. -DBUILD_TESTING=OFF
sudo cmake --build build/ --target install
rm -r build/