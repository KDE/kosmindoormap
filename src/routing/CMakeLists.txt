# SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
# SPDX-License-Identifier: BSD-2-Clause

add_library(KOSMIndoorRouting)
target_sources(KOSMIndoorRouting PRIVATE
    navmesh.cpp
    navmeshbuilder.cpp
    navmeshtransform.cpp
    routingjob.cpp
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
if (TARGET RecastNavigation::Recast AND TARGET RecastNavigation::Detour)
    target_compile_definitions(KOSMIndoorRouting PRIVATE HAVE_RECAST=1)
    target_link_libraries(KOSMIndoorRouting PRIVATE
        RecastNavigation::Recast
        RecastNavigation::Detour
        # RecastNavigation::DebugUtils
    )
else()
    target_compile_definitions(KOSMIndoorRouting PRIVATE HAVE_RECAST=0)
endif()

ecm_generate_headers(KOSMIndoorRouting_FORWARDING_HEADERS
    HEADER_NAMES
        NavMesh
        NavMeshBuilder
        NavMeshTransform
        RoutingJob
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
