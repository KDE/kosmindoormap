# SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>
# SPDX-License-Identifier: BSD-3-Clause

if (TARGET KOpeningHours)
    set(HAVE_KOPENINGHOURS TRUE)
endif()
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config-kosmindoormap.h.in ${CMAKE_CURRENT_BINARY_DIR}/config-kosmindoormap.h)

flex_target(mapcssscanner
    style/mapcsslexer.l
    ${CMAKE_CURRENT_BINARY_DIR}/mapcsslexer.cpp
    DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/mapcssscanner.h
    COMPILE_FLAGS "--nounistd"
)
bison_target(mapcssparser
    style/mapcssparser.y
    ${CMAKE_CURRENT_BINARY_DIR}/mapcssparser_p.cpp
    DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/mapcssparser_p.h
)
add_flex_bison_dependency(mapcssscanner mapcssparser)
set_source_files_properties(${BISON_mapcssparser_OUTPUTS} ${FLEX_mapcssscanner_OUTPUTS} PROPERTIES SKIP_AUTOMOC ON)

ecm_qt_declare_logging_category(logging_srcs HEADER logging.h IDENTIFIER KOSMIndoorMap::Log CATEGORY_NAME org.kde.kosmindoormap)
ecm_qt_declare_logging_category(logging_srcs HEADER render-logging.h IDENTIFIER KOSMIndoorMap::RenderLog CATEGORY_NAME org.kde.kosmindoormap.renderer)
ecm_qt_declare_logging_category(logging_srcs HEADER reassembly-logging.h IDENTIFIER KOSMIndoorMap::ReassemblyLog CATEGORY_NAME org.kde.kosmindoormap.reassembly)

add_library(KOSMIndoorMap
    ${logging_srcs}
    assets/assets.qrc

    content/equipmentmodel.cpp
    content/floorlevelmodel.cpp
    content/gatemodel.cpp
    content/platform.cpp
    content/platformfinder.cpp
    content/platformmodel.cpp

    loader/boundarysearch.cpp
    loader/levelparser.cpp
    loader/mapdata.cpp
    loader/maploader.cpp
    loader/marblegeometryassembler.cpp
    loader/tilecache.cpp

    renderer/hitdetector.cpp
    renderer/painterrenderer.cpp
    renderer/stackblur.cpp
    renderer/view.cpp

    scene/iconloader.cpp
    scene/openinghourscache.cpp
    scene/overlaysource.cpp
    scene/penwidthutil.cpp
    scene/scenecontroller.cpp
    scene/scenegeometry.cpp
    scene/scenegraph.cpp
    scene/scenegraphitem.cpp

    style/mapcsscondition.cpp
    style/mapcssdeclaration.cpp
    style/mapcssparser.cpp
    style/mapcssresult.cpp
    style/mapcssrule.cpp
    style/mapcssselector.cpp
    style/mapcssstate.cpp
    style/mapcssstyle.cpp
    ${BISON_mapcssparser_OUTPUTS}
    ${FLEX_mapcssscanner_OUTPUTS}
)

generate_export_header(KOSMIndoorMap BASE_NAME KOSMIndoorMap)
set_target_properties(KOSMIndoorMap PROPERTIES
    VERSION ${KOSMINDOORMAP_VERSION_STRING}
    SOVERSION ${KOSMINDOORMAP_SOVERSION}
    EXPORT_NAME KOSMIndoorMap
)

target_include_directories(KOSMIndoorMap PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>)
target_include_directories(KOSMIndoorMap PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>)
target_link_libraries(KOSMIndoorMap
    PUBLIC Qt5::Gui KOSM
    PRIVATE Qt5::Network Qt5::CorePrivate
)
if (TARGET KOpeningHours)
    target_link_libraries(KOSMIndoorMap PRIVATE KOpeningHours)
endif()

ecm_generate_headers(KOSMIndoorMap_Content_FORWARDING_HEADERS
    HEADER_NAMES
        EquipmentModel
        FloorLevelModel
        GateModel
        Platform
        PlatformModel
    PREFIX KOSMIndoorMap
    REQUIRED_HEADERS KOSMIndoorMap_Content_HEADERS
    RELATIVE content
)
ecm_generate_headers(KOSMIndoorMap_Loader_FORWARDING_HEADERS
    HEADER_NAMES
        MapLoader
        MapData
    PREFIX KOSMIndoorMap
    REQUIRED_HEADERS KOSMIndoorMap_Loader_HEADERS
    RELATIVE loader
)
ecm_generate_headers(KOSMIndoorMap_Renderer_FORWARDING_HEADERS
    HEADER_NAMES
        HitDetector
        PainterRenderer
        View
    PREFIX KOSMIndoorMap
    REQUIRED_HEADERS KOSMIndoorMap_Renderer_HEADERS
    RELATIVE renderer
)
ecm_generate_headers(KOSMIndoorMap_Scene_FORWARDING_HEADERS
    HEADER_NAMES
        OverlaySource
        SceneController
        SceneGraph
        SceneGraphItem
    PREFIX KOSMIndoorMap
    REQUIRED_HEADERS KOSMIndoorMap_Scene_HEADERS
    RELATIVE scene
)
ecm_generate_headers(KOSMIndoorMap_Style_FORWARDING_HEADERS
    HEADER_NAMES
        MapCSSParser
        MapCSSStyle
    PREFIX KOSMIndoorMap
    REQUIRED_HEADERS KOSMIndoorMap_Style_HEADERS
    RELATIVE style
)

install(TARGETS KOSMIndoorMap EXPORT KOSMIndoorMapTargets ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES
    ${KOSMIndoorMap_Content_FORWARDING_HEADERS}
    ${KOSMIndoorMap_Loader_FORWARDING_HEADERS}
    ${KOSMIndoorMap_Renderer_FORWARDING_HEADERS}
    ${KOSMIndoorMap_Scene_FORWARDING_HEADERS}
    ${KOSMIndoorMap_Style_FORWARDING_HEADERS}
    DESTINATION ${KDE_INSTALL_INCLUDEDIR}/KOSMIndoorMap
)
install(FILES
    ${KOSMIndoorMap_Content_HEADERS}
    ${KOSMIndoorMap_Loader_HEADERS}
    ${KOSMIndoorMap_Renderer_HEADERS}
    ${KOSMIndoorMap_Scene_HEADERS}
    ${KOSMIndoorMap_Style_HEADERS}
    ${CMAKE_CURRENT_BINARY_DIR}/kosmindoormap_export.h
    DESTINATION ${KDE_INSTALL_INCLUDEDIR}/kosmindoormap
)
install(FILES org_kde_kosmindoormap.categories DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR})
