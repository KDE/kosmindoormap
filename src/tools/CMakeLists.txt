# SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>
# SPDX-License-Identifier: BSD-3-Clause

add_executable(osm-download-data osm-download-data.cpp)
target_link_libraries(osm-download-data KOSMIndoorMap)
if (TARGET KOSM_pbfioplugin)
    target_compile_definitions(osm-download-data PRIVATE -DHAVE_OSM_PBF_SUPPORT=1)
    target_link_libraries(osm-download-data KOSM_pbfioplugin)
else()
    target_compile_definitions(osm-download-data PRIVATE -DHAVE_OSM_PBF_SUPPORT=0)
endif()

add_executable(marble-geometry-assembler marble-geometry-assembler.cpp)
target_link_libraries(marble-geometry-assembler KOSMIndoorMap)
if (TARGET KOSM_pbfioplugin)
    target_compile_definitions(marble-geometry-assembler PRIVATE -DHAVE_OSM_PBF_SUPPORT=1)
    target_link_libraries(marble-geometry-assembler KOSM_pbfioplugin)
else()
    target_compile_definitions(marble-geometry-assembler PRIVATE -DHAVE_OSM_PBF_SUPPORT=0)
endif()

if (TARGET KOSMIndoorRouting)
    add_executable(navmesh-dump navmesh-dump.cpp)
    target_link_libraries(navmesh-dump KOSMIndoorRouting)
endif()
