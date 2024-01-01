# SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
# SPDX-License-Identifier: BSD-2-Clause

add_library(KOSMIndoorRouting)
target_sources(KOSMIndoorRouting PRIVATE
    navmeshbuilder.cpp
    navmeshtransform.cpp
    assets.qrc
)

ecm_qt_declare_logging_category(KOSMIndoorRouting
    HEADER logging.h
    IDENTIFIER KOSMIndoorRouting::Log
    CATEGORY_NAME org.kde.kosmindoorrouting
    DESCRIPTION "KOSMIndoorRouting"
    EXPORT KOSMINDOORMAP
)

generate_export_header(KOSMIndoorRouting BASE_NAME KOSMIndoorRouting)
set_target_properties(KOSMIndoorRouting PROPERTIES
    VERSION ${KOSMINDOORMAP_VERSION}
    SOVERSION ${KOSMINDOORMAP_SOVERSION}
    EXPORT_NAME KOSMIndoorRouting
)

target_include_directories(KOSMIndoorRouting PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>)
target_link_libraries(KOSMIndoorRouting
    PUBLIC Qt::Core KOSM KOSMIndoorMap
    PRIVATE Qt::GuiPrivate
)

ecm_generate_headers(KOSMIndoorRouting_FORWARDING_HEADERS
    HEADER_NAMES
        NavMeshBuilder
    PREFIX KOSMIndoorRouting
    REQUIRED_HEADERS KOSMIndoorRouting_HEADERS
)

install(TARGETS KOSMIndoorRouting EXPORT KOSMIndoorMapTargets ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES
    ${KOSMIndoorRouting_FORWARDING_HEADERS}
    DESTINATION ${KDE_INSTALL_INCLUDEDIR}/KOSMIndoorRouting
)
install(FILES
    ${KOSMIndoorRouting_HEADERS}
    ${CMAKE_CURRENT_BINARY_DIR}/kosmindoorrouting_export.h
    DESTINATION ${KDE_INSTALL_INCLUDEDIR}/kosmindoorrouting
)
