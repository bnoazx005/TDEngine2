#!/bin/bash

sudo add-apt-repository -y ppa:ubuntu-toolchain-r/test
sudo apt-get update -qq

sudo apt-get install -qq g++-7
sudo update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-7 90

sudo apt-get install -qq unzip

sudo apt-get install -qq build-essential xorg-dev libgl1-mesa-dev libglew-dev libc++-dev libxmu-dev libxi-dev

CMAKE_VERSION=3.10.1
CMAKE_VERSION_DIR=v3.10

CMAKE_OS=Linux-x86_64
CMAKE_TAR=cmake-$CMAKE_VERSION-$CMAKE_OS.tar.gz
CMAKE_URL=http://www.cmake.org/files/$CMAKE_VERSION_DIR/$CMAKE_TAR
CMAKE_DIR=$(pwd)/cmake-$CMAKE_VERSION

wget --quiet $CMAKE_URL
mkdir -p $CMAKE_DIR
tar --strip-components=1 -xzf $CMAKE_TAR -C $CMAKE_DIR
export PATH=$CMAKE_DIR/bin:$PATH

GLEW_VERSION=2.1.0

GLEW_ARCHIVE=glew-$GLEW_VERSION.zip
GLEW_URL=https://sourceforge.net/projects/glew/files/glew/$GLEW_VERSION/$GLEW_ARCHIVE
GLEW_DIR=$(pwd)/glew-$GLEW_VERSION

wget --quiet $GLEW_URL
mkdir -p $GLEW_DIR
unzip $GLEW_ARCHIVE -d .

#build GLEW
pushd $GLEW_DIR && make && sudo make install && make clean && popd