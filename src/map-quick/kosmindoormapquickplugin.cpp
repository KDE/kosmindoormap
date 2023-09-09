/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kosmindoormapquickplugin.h"

#include "amenitymodel.h"
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
    Q_INIT_RESOURCE(assets);

    qRegisterMetaType<MapData>();
    qRegisterMetaType<OSMAddress>();
    qRegisterMetaType<OSMElement>();
    qRegisterMetaType<Platform>();
    qRegisterMetaType<Platform::Mode>();

    qmlRegisterUncreatableMetaObject(Platform::staticMetaObject, "org.kde.kosmindoormap", 1, 0, "Platform", {});

    qmlRegisterType<AmenityModel>("org.kde.kosmindoormap", 1, 0, "AmenityModel");
    qmlRegisterType<FloorLevelChangeModel>("org.kde.kosmindoormap", 1, 0, "FloorLevelChangeModel");
    qmlRegisterType<MapItem>("org.kde.kosmindoormap", 1, 0, "MapItemImpl");
    qmlRegisterType<OSMElementInformationModel>("org.kde.kosmindoormap", 1, 0, "OSMElementInformationModel");
    qmlRegisterType<EquipmentModel>("org.kde.kosmindoormap", 1, 0, "EquipmentModel");
    qmlRegisterType<GateModel>("org.kde.kosmindoormap", 1, 0, "GateModel");
    qmlRegisterType<PlatformModel>("org.kde.kosmindoormap", 1, 0, "PlatformModel");
}
