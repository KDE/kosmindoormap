#!/bin/bash
# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: 2020-2022 Volker Krause <vkrause@kde.org>
set -e
set -x

function build-static-qt-module() {
    local module=$1
    shift

    mkdir -p $BUILD_ROOT
    mkdir -p $STAGING_ROOT
    pushd $BUILD_ROOT

    git clone --branch kde/5.15 --depth 1 https://invent.kde.org/qt/qt/$module
    cd $module
    mkdir build
    cd build
    if [ $module == "qtbase" ]; then
        ../configure -prefix $STAGING_ROOT $@
    else
        $STAGING_ROOT/bin/qmake .. $@
    fi
    make -j 4
    make install

    popd
}

export OPENSSL_LIBS="-L$STAGING_ROOT/lib -lssl -lcrypto -pthread -ldl"

build-static-qt-module qtbase \
    -release -optimize-size \
    -qpa offscreen \
    -no-pch \
    -no-icu \
    -no-dbus \
    -no-glib \
    -no-xcb -no-opengl -no-feature-vulkan \
    -no-feature-sql \
    -no-widgets \
    -no-gui \
    -no-feature-concurrent -no-feature-future -no-feature-sharedmemory -no-feature-systemsemaphore \
    -no-feature-statemachine \
    -no-feature-desktopservices \
    -no-feature-proxymodel -no-feature-stringlistmodel \
    -no-feature-testlib \
    -ssl -openssl-linked -I $STAGING_ROOT/include -L $STAGING_ROOT/lib \
    -static -confirm-license -opensource -make libs -make tools

# Patch .prl files to use static zlib
for i in `find $STAGING_ROOT -name "*.prl"`; do
    sed -i -e 's,-lz,/usr/lib64/libz.a,g' $i
done
