#!/usr/bin/env bash

set -x

PKGBUILD=$(date '+%y%m%d%H%M%S')
# SRC=$(pwd)

INSTALL_DIR=$(readlink install)
BUILDX=$(basename -s .install "${INSTALL_DIR}")
echo "${BUILDX}"
BUILD=$(echo "${BUILDX}" | tr _ -)
echo "${BUILD}"

PACKAGE_NAME=uhmitest
PACKAGE_VERSION=0.1

rm -rf "${INSTALL_DIR}/DEBIAN"
mkdir "${INSTALL_DIR}/DEBIAN"

cat <<EOF > "${INSTALL_DIR}/DEBIAN/control"
Package: ${PACKAGE_NAME}
Version: ${PACKAGE_VERSION}-${BUILD}.${PKGBUILD}
Section: base
Priority: optional
Architecture: amd64
Depends: libgles2 (>= 1.3.1), libegl1 (>= 1.3.1)
Maintainer: Aleksei Makarov <alm@opensynergy.com>
Description: Test program for uhmi
EOF


FILENAME=./xchg/"${PACKAGE_NAME}_${PACKAGE_VERSION}-${BUILD}.${PKGBUILD}.deb"
rm -f ./xchg/"${PACKAGE_NAME}"*.deb
dpkg-deb --build "${INSTALL_DIR}" "${FILENAME}"
