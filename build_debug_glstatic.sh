#!/usr/bin/env bash

DATE=$(date '+%y%m%d%H%M%S')
PWD=$(pwd)

INSTALL_DIR=$PWD/${DATE}_gl_static_linking.install
BUILD_DIR=$PWD/${DATE}_gl_static_linking.build

echo "install: $INSTALL_DIR, build: $BUILD_DIR"

mkdir -p "$INSTALL_DIR"
# cmake --install-prefix="$INSTALL_DIR" -S . -B "$BUILD_DIR"
cmake -D CMAKE_INSTALL_PREFIX="$INSTALL_DIR" -D CMAKE_BUILD_TYPE=Debug -D GL_STATIC_LINKING=Yes -S . -B "$BUILD_DIR"

ln -fs -T "$INSTALL_DIR" install
ln -fs -T "$BUILD_DIR" build

cd "$BUILD_DIR" || exit

cmake --build . --verbose
cmake --install . --verbose
