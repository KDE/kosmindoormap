# SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.16)

# KDE Application Version, managed by release script
set (RELEASE_SERVICE_VERSION_MAJOR "24")
set (RELEASE_SERVICE_VERSION_MINOR "07")
set (RELEASE_SERVICE_VERSION_MICRO "80")
set (RELEASE_SERVICE_VERSION "${RELEASE_SERVICE_VERSION_MAJOR}.${RELEASE_SERVICE_VERSION_MINOR}.${RELEASE_SERVICE_VERSION_MICRO}")
project(KOSMIndoorMap VERSION ${RELEASE_SERVICE_VERSION})

option(BUILD_STANDALONE_APP "Build and install the stand-alone test/demo app." OFF)
option(BUILD_TOOLS_ONLY "Build only the command-line tools." OFF)

find_package(ECM 6.0 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake ${ECM_MODULE_PATH})
set(CMAKE_CXX_STANDARD 20)

include(KDECompilerSettings NO_POLICY_SCOPE)
include(ECMAddTests)
include(ECMGenerateHeaders)
include(ECMQtDeclareLoggingCategory)
include(ECMDeprecationSettings)
include(ECMSetupVersion)
include(FeatureSummary)
include(KDEInstallDirs)
include(KDECMakeSettings)
include(GenerateExportHeader)
include(ECMDeprecationSettings)
include(ECMAddAndroidApk)
include(ECMQmlModule)

ecm_setup_version(PROJECT VARIABLE_PREFIX KOSMINDOORMAP
    VERSION_HEADER kosmindoormap_version.h
    PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KOSMIndoorMapConfigVersion.cmake"
    SOVERSION 1
)

set(QT_MIN_VERSION 6.5)
find_package(Qt6 ${QT_MIN_VERSION} REQUIRED COMPONENTS Core Network)

set(KPublicTransport_DEPENDENCY_TYPE REQUIRED)
if (NOT USE_KPUBLICTRANSPORT AND NOT BUILD_STANDALONE_APP)
    set(KPublicTransport_DEPENDENCY_TYPE RECOMMENDED)
endif()

if (NOT BUILD_TOOLS_ONLY)
    find_package(Qt6 ${QT_MIN_VERSION} REQUIRED COMPONENTS Quick)
    find_package(KF6 REQUIRED COMPONENTS I18n)
    find_package(KPublicTransport)
    set_package_properties(KPublicTransport PROPERTIES TYPE ${KPublicTransport_DEPENDENCY_TYPE} PURPOSE "Integration of public transport realtime data.")
    find_package(KOpeningHours)
    set_package_properties(KOpeningHours PROPERTIES TYPE REQUIRED PURPOSE "Highlight currently open amenities/shops/etc.")
    if (ANDROID)
        find_package(Gradle REQUIRED)
    else()
        find_package(KF6 OPTIONAL_COMPONENTS CoreAddons Service)
    endif()

    find_package(recastnavigation)
    set_package_properties(recastnavigation PROPERTIES TYPE RECOMMENDED PURPOSE "Navigation engine." URL "https://recastnav.com/")
endif()

if (BUILD_TESTING)
    find_package(Qt6 ${QT_MIN_VERSION} REQUIRED COMPONENTS Test Widgets)
endif()

if (BUILD_STANDALONE_APP)
    if (NOT ANDROID)
        find_package(Qt6 ${QT_MIN_VERSION} REQUIRED COMPONENTS Widgets)
    else()
        find_package(KF6 6.3 REQUIRED COMPONENTS Kirigami)
        find_package(Qt6 ${QT_MIN_VERSION} REQUIRED COMPONENTS QuickControls2 Svg)
        find_package(OpenSSL REQUIRED)
        if (NOT DEFINED BREEZEICONS_DIR AND EXISTS ${CMAKE_SOURCE_DIR}/../breeze-icons)
            set(BREEZEICONS_DIR ${CMAKE_SOURCE_DIR}/../breeze-icons)
        endif()
    endif()
endif()

find_package(ZLIB REQUIRED)
find_package(OsmTools)
set_package_properties(OsmTools PROPERTIES TYPE OPTIONAL PURPOSE "Needed only for regenereating line metadata tables (ie. you most likely don't need this)")

if (NOT BUILD_TOOLS_ONLY)
    find_package(FLEX REQUIRED)
    set_package_properties(FLEX PROPERTIES PURPOSE "MapCSS parser for indoor map rendering.")
    find_package(BISON REQUIRED)
    set_package_properties(BISON PROPERTIES PURPOSE "MapCSS parser for indoor map rendering.")
endif()

if (NOT WIN32)
    # try upstream cmake config first
    set(protobuf_MODULE_COMPATIBLE ON)
    find_package(Protobuf CONFIG)
    if(NOT Protobuf_FOUND)
    # fall back to cmake's protobuf module
       find_package(Protobuf)
    endif()
    set_package_properties(Protobuf PROPERTIES TYPE OPTIONAL PURPOSE "Parsing of OSM PBF files.")
endif()

add_definitions(-DQT_NO_CONTEXTLESS_CONNECT)
ecm_set_disabled_deprecation_versions(QT 6.5 KF 5.240.0)

add_subdirectory(src)
if (BUILD_TESTING)
    add_subdirectory(autotests)
    add_subdirectory(tests)
endif()

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)

set(CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/KOSMIndoorMap")
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
if (NOT BUILD_TOOLS_ONLY)
    ki18n_install(po)
endif()
