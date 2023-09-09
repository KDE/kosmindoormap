# SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>
# SPDX-License-Identifier: BSD-3-Clause

add_library(KOSMIndoorMapQuick STATIC
    amenitymodel.cpp
    assets.qrc
    floorlevelchangemodel.cpp
    localization.cpp
    mapitem.cpp
    osmaddress.cpp
    osmelement.cpp
    osmelementinformationmodel.cpp
)
set_target_properties(KOSMIndoorMapQuick PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_link_libraries(KOSMIndoorMapQuick PUBLIC
    Qt::Quick
    KF${KF_MAJOR_VERSION}::I18n
    KF${KF_MAJOR_VERSION}::I18nLocaleData
    KOSMIndoorMap
)

ecm_qt_declare_logging_category(KOSMIndoorMapQuick
    HEADER logging.h
    IDENTIFIER KOSMIndoorMap::Log
    CATEGORY_NAME org.kde.kosmindoormap.quick
    DESCRIPTION "KOSMIndoorMap (Quick Bindings)"
    EXPORT KOSMINDOORMAP
)

set(kosmindoormapquickplugin_qml
    qmldir
    IndoorMap.qml
    IndoorMapAttributionLabel.qml
    IndoorMapScale.qml
)

add_library(kosmindoormapquickplugin kosmindoormapquickplugin.cpp)
target_link_libraries(kosmindoormapquickplugin
    KOSMIndoorMapQuick
)

# make examples work without installation
if (NOT CMAKE_VERSION VERSION_LESS 3.14)
    set_property(TARGET kosmindoormapquickplugin PROPERTY LIBRARY_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/org/kde/kosmindoormap)
    file(MAKE_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/org/kde/kosmindoormap/)
    foreach(f IN LISTS kosmindoormapquickplugin_qml)
        file(CREATE_LINK ${CMAKE_CURRENT_SOURCE_DIR}/${f} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/org/kde/kosmindoormap/${f} SYMBOLIC)
    endforeach()
else()
    message(WARNING "CMake is too old, cannot setup examples to work without installation")
endif()

install(TARGETS kosmindoormapquickplugin DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/kosmindoormap)
install(FILES ${kosmindoormapquickplugin_qml} DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/kosmindoormap)
