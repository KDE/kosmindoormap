# SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>
# SPDX-License-Identifier: BSD-3-Clause

add_definitions(-DTRANSLATION_DOMAIN=\"kosmindoormap\")

add_subdirectory(osm)
add_subdirectory(map)
if (NOT BUILD_TOOLS_ONLY)
    add_subdirectory(routing)
endif()
if (TARGET Qt::Quick)
    include(ECMQmlModule)

    add_subdirectory(editor)
    add_subdirectory(wikidata)
    add_subdirectory(map-quick)
endif()
if (TARGET Qt::Quick AND TARGET KPublicTransport)
    add_subdirectory(map-publictransport-integration)
    add_subdirectory(routing-quick)
endif()
if (BUILD_STANDALONE_APP)
    add_subdirectory(app)
endif()
if (NOT CROSS_COMPILING)
    add_subdirectory(tools)
endif()

ecm_qt_install_logging_categories(
    EXPORT KOSMINDOORMAP
    FILE org_kde_kosmindoormap.categories
    DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR}
)
