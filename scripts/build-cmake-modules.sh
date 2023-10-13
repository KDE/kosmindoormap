#!/bin/bash
# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: 2020-2022 Volker Krause <vkrause@kde.org>
set -e
set -x

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
        git clone $repo $module
        cd $module
        git checkout $version
    else
        # already checked out, so we assume it's the current module set up by Gitlab for us
        cd $module
    fi

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
    make install

    popd
}

function build_kde_module() {
    local module=$1
    shift
    build_cmake_module https://invent.kde.org/$module $module master $@
}

function build_kf_module() {
    local module=$1
    shift
    build_cmake_module https://invent.kde.org/$module $module kf5 $@
}

build_cmake_module https://github.com/protocolbuffers/protobuf protobuf 21.x \
    -Dprotobuf_BUILD_TESTS=OFF

# KDE Frameworks
build_kf_module frameworks/extra-cmake-modules

export CXXFLAGS="-static-libstdc++ -static-libgcc"
build_kde_module $CI_PROJECT_PATH -DBUILD_TOOLS_ONLY=ON
