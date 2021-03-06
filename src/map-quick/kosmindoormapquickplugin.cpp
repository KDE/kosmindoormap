/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kosmindoormapquickplugin.h"
#include "floorlevelchangemodel.h"
#include "mapitem.h"
#include "osmaddress.h"
#include "osmelement.h"
#include "osmelementinformationmodel.h"

#include <KOSMIndoorMap/EquipmentModel>
#include <KOSMIndoorMap/GateModel>
#include <KOSMIndoorMap/MapData>
#include <KOSMIndoorMap/PlatformModel>

using namespace KOSMIndoorMap;

void KOSMIndoorMapQuickPlugin::registerTypes(const char *uri)
{
    Q_UNUSED(uri);
    qRegisterMetaType<OSMAddress>();
    qRegisterMetaType<OSMElement>();
    qRegisterMetaType<Platform::Mode>();

    qmlRegisterUncreatableType<MapData>("org.kde.kosmindoormap", 1, 0, "MapData", {});
    qmlRegisterUncreatableType<OSMAddress>("org.kde.kosmindoormap", 1, 0, "OSMAddress", {});
    qmlRegisterUncreatableType<OSMElement>("org.kde.kosmindoormap", 1, 0, "OSMElement", {});
    qmlRegisterUncreatableType<Platform>("org.kde.kosmindoormap", 1, 0, "Platform", {});

    qmlRegisterType<FloorLevelChangeModel>("org.kde.kosmindoormap", 1, 0, "FloorLevelChangeModel");
    qmlRegisterType<MapItem>("org.kde.kosmindoormap", 1, 0, "MapItemImpl");
    qmlRegisterType<OSMElementInformationModel>("org.kde.kosmindoormap", 1, 0, "OSMElementInformationModel");
    qmlRegisterType<EquipmentModel>("org.kde.kosmindoormap", 1, 0, "EquipmentModel");
    qmlRegisterType<GateModel>("org.kde.kosmindoormap", 1, 0, "GateModel");
    qmlRegisterType<PlatformModel>("org.kde.kosmindoormap", 1, 0, "PlatformModel");
}
