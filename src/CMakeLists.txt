# SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>
# SPDX-License-Identifier: BSD-3-Clause

add_definitions(-DTRANSLATION_DOMAIN=\"kosmindoormap\")

add_subdirectory(osm)
add_subdirectory(map)
if (TARGET Qt${QT_MAJOR_VERSION}::Quick)
    add_subdirectory(map-quick)
    add_subdirectory(map-publictransport-integration)
endif()
if ((NOT ANDROID AND TARGET Qt${QT_MAJOR_VERSION}::Widgets) OR BUILD_STANDALONE_APP)
    add_subdirectory(app)
endif()
if (NOT CROSS_COMPILING)
    add_subdirectory(tools)
endif()
