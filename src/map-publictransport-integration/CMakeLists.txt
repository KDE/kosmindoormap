# SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>
# SPDX-License-Identifier: BSD-3-Clause

ecm_add_qml_module(kosmindoormap_kpublictransport_integration_plugin
    URI "org.kde.kosmindoormap.kpublictransport"
    GENERATE_PLUGIN_SOURCE
    DEPENDENCIES
        "org.kde.kosmindoormap"
)
target_sources(kosmindoormap_kpublictransport_integration_plugin PRIVATE
    locationqueryoverlayproxymodel.cpp
    realtimeequipmentmodel.cpp
)
target_link_libraries(kosmindoormap_kpublictransport_integration_plugin PRIVATE
    Qt::Qml
    KOSMIndoorMap
    KPublicTransport
)
ecm_finalize_qml_module(kosmindoormap_kpublictransport_integration_plugin)
