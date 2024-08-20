# SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>
# SPDX-License-Identifier: BSD-3-Clause

ecm_add_qml_module(kosmindoormap_kpublictransport_integration_plugin
    URI "org.kde.kosmindoormap.kpublictransport"
    GENERATE_PLUGIN_SOURCE
)
target_sources(kosmindoormap_kpublictransport_integration_plugin PRIVATE
    locationqueryoverlayproxymodel.cpp
    realtimeequipmentmodel.cpp
)
target_include_directories(kosmindoormap_kpublictransport_integration_plugin PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../map/content;${CMAKE_CURRENT_SOURCE_DIR}/../map/loader;${CMAKE_CURRENT_SOURCE_DIR}/../map/scene>")
target_link_libraries(kosmindoormap_kpublictransport_integration_plugin PRIVATE
    Qt::Qml
    KOSMIndoorMap
    KPublicTransport
)
ecm_finalize_qml_module(kosmindoormap_kpublictransport_integration_plugin)
