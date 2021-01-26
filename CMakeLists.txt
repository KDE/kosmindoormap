# SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.5)

# KDE Application Version, managed by release script
set (RELEASE_SERVICE_VERSION_MAJOR "21")
set (RELEASE_SERVICE_VERSION_MINOR "03")
set (RELEASE_SERVICE_VERSION_MICRO "70")
set (RELEASE_SERVICE_VERSION "${RELEASE_SERVICE_VERSION_MAJOR}.${RELEASE_SERVICE_VERSION_MINOR}.${RELEASE_SERVICE_VERSION_MICRO}")
project(KOSMIndoorMap VERSION ${RELEASE_SERVICE_VERSION})

option(BUILD_STANDALONE_APP "Build and install the stand-alone test/demo app." OFF)

find_package(ECM 5.59 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

include(KDEFrameworkCompilerSettings NO_POLICY_SCOPE)
include(ECMAddTests)
include(ECMGenerateHeaders)
include(ECMQtDeclareLoggingCategory)
include(ECMSetupVersion)
include(FeatureSummary)
include(KDEInstallDirs)
include(KDECMakeSettings)
include(GenerateExportHeader)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
ecm_setup_version(PROJECT VARIABLE_PREFIX KOSMINDOORMAP
    VERSION_HEADER kosmindoormap_version.h
    PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KOSMIndoorMapConfigVersion.cmake"
    SOVERSION 1
)

find_package(Qt5 5.14 REQUIRED COMPONENTS Quick OPTIONAL_COMPONENTS Widgets)
find_package(KF5 REQUIRED COMPONENTS I18n)
find_package(KPublicTransport REQUIRED)
find_package(KOpeningHours)
set_package_properties(KOpeningHours PROPERTIES TYPE OPTIONAL PURPOSE "Highlight currently open amenities/shops/etc.")

if (BUILD_TESTING)
    find_package(Qt5 5.14 REQUIRED COMPONENTS Test Widgets)
endif()

if (BUILD_STANDALONE_APP)
    if (NOT ANDROID)
        find_package(Qt5 5.14 REQUIRED COMPONENTS Widgets)
    else()
        find_package(KF5 REQUIRED COMPONENTS Kirigami2)
        find_package(Qt5 5.14 REQUIRED COMPONENTS QuickControls2 Svg)
        find_package(OpenSSL REQUIRED)
        if (NOT DEFINED BREEZEICONS_DIR AND EXISTS ${CMAKE_SOURCE_DIR}/../breeze-icons)
            set(BREEZEICONS_DIR ${CMAKE_SOURCE_DIR}/../breeze-icons)
        endif()
    endif()
endif()

find_package(ZLIB REQUIRED)
find_package(OsmTools)
set_package_properties(OsmTools PROPERTIES TYPE OPTIONAL PURPOSE "Needed only for regenereating line metadata tables (ie. you most likely don't need this)")

find_package(FLEX REQUIRED)
set_package_properties(FLEX PROPERTIES PURPOSE "MapCSS parser for indoor map rendering.")
find_package(BISON REQUIRED)
set_package_properties(BISON PROPERTIES PURPOSE "MapCSS parser for indoor map rendering.")

if (NOT WIN32)
    find_package(Protobuf)
    set_package_properties(Protobuf PROPERTIES TYPE OPTIONAL PURPOSE "Parsing of OSM PBF files.")
endif()

add_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0x050e00)
add_definitions(-DKF_DISABLE_DEPRECATED_BEFORE_AND_AT=0x054400)
add_definitions(-DQT_NO_FOREACH)

add_subdirectory(src)
if (BUILD_TESTING)
    add_subdirectory(autotests)
    add_subdirectory(tests)
endif()

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)

set(CMAKECONFIG_INSTALL_DIR "${CMAKECONFIG_INSTALL_PREFIX}/KOSMIndoorMap")
configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/KOSMIndoorMapConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/KOSMIndoorMapConfig.cmake"
    INSTALL_DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
)

install(FILES
        "${CMAKE_CURRENT_BINARY_DIR}/KOSMIndoorMapConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/KOSMIndoorMapConfigVersion.cmake"
    DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
    COMPONENT Devel
)
install(EXPORT KOSMIndoorMapTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}" FILE KOSMIndoorMapTargets.cmake)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kosmindoormap_version.h DESTINATION ${KDE_INSTALL_INCLUDEDIR} COMPONENT Devel)
ki18n_install(po)
