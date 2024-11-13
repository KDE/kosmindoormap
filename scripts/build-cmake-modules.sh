#!/bin/bash
# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: 2020-2022 Volker Krause <vkrause@kde.org>
set -e
set -x

PROTOBUF_VERSION=21.x
KF_VERSION="v6.8.0"
GEAR_VERSION="master"

function build_cmake_module() {
    local repo=$1
    shift
    local module=$1
    shift
    local version=$1
    shift

    mkdir -p $BUILD_ROOT
    mkdir -p $STAGING_ROOT
    pushd $BUILD_ROOT

    if ! [ -d $BUILD_ROOT/$module ]; then
        git clone --branch $version --depth 1 $repo $module
    fi
    cd $module

    mkdir build
    cd build
    cmake -DBUILD_SHARED=ON \
        -DBUILD_TESTING=OFF -DBUILD_SHARED_LIBS=OFF \
        -DCMAKE_PREFIX_PATH=$STAGING_ROOT \
        -DCMAKE_INSTALL_PREFIX=$STAGING_ROOT \
        -DCMAKE_EXE_LINKER_FLAGS="-Wl,--as-needed" \
        -DZLIB_USE_STATIC_LIBS=ON \
        $@ -DCMAKE_BUILD_TYPE=Release ..

    make -j 4
    make install/fast

    popd
}

function build_kde_module() {
    local module=$1
    shift
    build_cmake_module https://invent.kde.org/$module $module $GEAR_VERSION $@
}

function build_kf_module() {
    local module=$1
    shift
    build_cmake_module https://invent.kde.org/$module $module $KF_VERSION $@
}

build_cmake_module https://github.com/protocolbuffers/protobuf protobuf $PROTOBUF_VERSION \
    -Dprotobuf_BUILD_TESTS=OFF

# KDE Frameworks
build_kf_module frameworks/extra-cmake-modules

export CXXFLAGS="-static-libstdc++ -static-libgcc"
build_kde_module $CI_PROJECT_PATH -DBUILD_TOOLS_ONLY=ON -DBUILD_WITH_QT6=ON
