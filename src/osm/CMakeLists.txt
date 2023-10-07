# SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>
# SPDX-License-Identifier: BSD-3-Clause

add_library(KOSM
    abstractreader.cpp
    abstractwriter.cpp
    datatypes.cpp
    datasetmergebuffer.cpp
    element.cpp
    geomath.cpp
    io.cpp
    languages.cpp
    o5mparser.cpp
    o5mwriter.cpp
    overpassquery.cpp
    overpassquerymanager.cpp
    pathutil.cpp
    stringpool.cpp
    ztile.cpp

    io/o5mplugin.cpp
)
generate_export_header(KOSM BASE_NAME KOSM)
set_target_properties(KOSM PROPERTIES
    VERSION ${KOSMINDOORMAP_VERSION}
    SOVERSION ${KOSMINDOORMAP_SOVERSION}
    EXPORT_NAME KOSM
)

target_include_directories(KOSM PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>")
target_link_libraries(KOSM PUBLIC Qt::Core PRIVATE Qt::Network)

ecm_generate_headers(KOSM_FORWARDING_HEADERS
    HEADER_NAMES
        Datatypes
        Element
    PREFIX KOSM
    REQUIRED_HEADERS KOSM_HEADERS
)

install(TARGETS KOSM EXPORT KOSMIndoorMapTargets ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES
    ${KOSM_FORWARDING_HEADERS}
    DESTINATION ${KDE_INSTALL_INCLUDEDIR}/KOSM
)
install(FILES
    ${KOSM_HEADERS}
    ${CMAKE_CURRENT_BINARY_DIR}/kosm_export.h
    internal.h
    stringpool.h
    DESTINATION ${KDE_INSTALL_INCLUDEDIR}/kosm
)

add_subdirectory(io)
