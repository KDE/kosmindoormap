# SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>
# SPDX-License-Identifier: BSD-3-Clause

add_library(KOSMIndoorMapQuick STATIC
    amenitymodel.cpp
    amenitysortfilterproxymodel.cpp
    assets.qrc
    floorlevelchangemodel.cpp
    localization.cpp
    mapitem.cpp
    osmaddress.cpp
    osmelement.cpp
    osmelementinformationmodel.cpp
    platformutil.cpp
    roommodel.cpp
    roomsortfilterproxymodel.cpp
)
set_target_properties(KOSMIndoorMapQuick PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_link_libraries(KOSMIndoorMapQuick PUBLIC
    Qt::Quick
    KF6::I18n
    KF6::I18nLocaleData
    KOSMIndoorMap
    KWikidata
)
qt_extract_metatypes(KOSMIndoorMapQuick)

ecm_qt_declare_logging_category(KOSMIndoorMapQuick
    HEADER logging.h
    IDENTIFIER KOSMIndoorMap::Log
    CATEGORY_NAME org.kde.kosmindoormap.quick
    DESCRIPTION "KOSMIndoorMap (Quick Bindings)"
    EXPORT KOSMINDOORMAP
)

ecm_add_qml_module(kosmindoormapquickplugin
    URI "org.kde.kosmindoormap"
    VERSION 1.0
    QML_FILES
        AmenityListDelegate.qml
        AmenitySearchDialog.qml
        FloorLevelSelector.qml
        IndoorMap.qml
        IndoorMapView.qml
        IndoorMapAttributionLabel.qml
        IndoorMapScale.qml
        OSMElementInformationDialog.qml
        OSMElementInformationDialogAddressDelegate.qml
        OSMElementInformationDialogImageDelegate.qml
        OSMElementInformationDialogLinkDelegate.qml
        OSMElementInformationDialogOpeningHoursDelegate.qml
        OSMElementInformationDialogStringDelegate.qml
        RoomSearchDialog.qml
    SOURCES
        mappointerevent.cpp
    NO_GENERATE_PLUGIN_SOURCE
    NO_PLUGIN_OPTIONAL
    CLASS_NAME "KOSMIndoorMapQuickPlugin"
    DEPENDENCIES
        "QtQuick.Controls"
        "QtQuick.Layouts"
        "org.kde.kirigami"
        "org.kde.kirigamiaddons.components"
        "org.kde.kopeninghours"
        "org.kde.contacts"
)
target_sources(kosmindoormapquickplugin PRIVATE
    kosmindoormapquickplugin.cpp
    types.h
)
target_link_libraries(kosmindoormapquickplugin PRIVATE
    KOSMIndoorMapQuick
)

ecm_finalize_qml_module(kosmindoormapquickplugin)
