# SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>
# SPDX-License-Identifier: BSD-3-Clause

add_executable(overpassql-cli overpassql-cli.cpp)
target_link_libraries(overpassql-cli KOSM KOSM_xmlioplugin)

add_executable(indoormap indoormap.cpp ../src/map/assets/assets.qrc)
target_compile_definitions(indoormap PRIVATE -DSOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(indoormap PRIVATE KOSMIndoorMap Qt::Widgets KOSM_xmlioplugin)
if (TARGET KOSM_pbfioplugin)
    target_compile_definitions(indoormap PRIVATE -DHAVE_OSM_PBF_SUPPORT=1)
    target_link_libraries(indoormap PRIVATE KOSM_pbfioplugin)
else()
    target_compile_definitions(indoormap PRIVATE -DHAVE_OSM_PBF_SUPPORT=0)
endif()
