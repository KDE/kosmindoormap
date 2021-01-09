# SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>
# SPDX-License-Identifier: BSD-3-Clause

add_definitions(-DTRANSLATION_DOMAIN=\"kosmindoormap\")

add_subdirectory(osm)
add_subdirectory(map)
if (TARGET Qt5::Quick)
    add_subdirectory(map-quick)
    add_subdirectory(map-publictransport-integration)
    add_subdirectory(app)
endif()
if (NOT CROSS_COMPILING)
    add_subdirectory(tools)
endif()
