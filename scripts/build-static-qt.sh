#!/bin/bash
# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: 2020-2022 Volker Krause <vkrause@kde.org>
set -e
set -x

QT_VERSION=6.6.2

function build-static-qt-module() {
    local module=$1
    shift

    mkdir -p $BUILD_ROOT
    mkdir -p $STAGING_ROOT
    pushd $BUILD_ROOT

    git clone --branch v$QT_VERSION --depth 1 https://invent.kde.org/qt/qt/$module
    cd $module
    mkdir build
    cd build
    if [ $module == "qtbase" ]; then
        ../configure -prefix $STAGING_ROOT $@
    else
        cmake -DCMAKE_PREFIX_PATH=$STAGING_ROOT -DCMAKE_INSTALL_PREFIX=$STAGING_ROOT .. $@
    fi
    make -j 4
    make install/fast

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
    -no-feature-concurrent -no-feature-future -no-feature-sharedmemory \
    -no-feature-desktopservices \
    -no-feature-proxymodel -no-feature-stringlistmodel \
    -no-feature-testlib \
    -ssl -openssl-linked -I $STAGING_ROOT/include -L $STAGING_ROOT/lib \
    -static -confirm-license -opensource -make libs -make tools
