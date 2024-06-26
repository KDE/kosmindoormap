/*
    SPDX-FileCopyrightText: 2024 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KOSMINDOORMAP_TYPES_H
#define KOSMINDOORMAP_TYPES_H

#include <KOSMIndoorMap/EquipmentModel>
#include <KOSMIndoorMap/GateModel>
#include <KOSMIndoorMap/PlatformModel>

#include "amenitymodel.h"
#include "amenitysortfilterproxymodel.h"
#include "floorlevelchangemodel.h"
#include "mapitem.h"
#include "osmelementinformationmodel.h"
#include "roommodel.h"
#include "roomsortfilterproxymodel.h"

#include <QQmlEngine>

// from base library
struct EquipmentModelForeign {
    Q_GADGET
    QML_NAMED_ELEMENT(EquipmentModel)
    QML_FOREIGN(KOSMIndoorMap::EquipmentModel)
};

struct GateModelForeign {
    Q_GADGET
    QML_NAMED_ELEMENT(GateModel)
    QML_FOREIGN(KOSMIndoorMap::GateModel)
};

struct PlatformModelForeign {
    Q_GADGET
    QML_NAMED_ELEMENT(PlatformModel)
    QML_FOREIGN(KOSMIndoorMap::PlatformModel)
};


// from QML library
struct AmenityModelForeign {
    Q_GADGET
    QML_NAMED_ELEMENT(AmenityModel)
    QML_FOREIGN(KOSMIndoorMap::AmenityModel)
};

struct AmenitySortFilterProxyModelForeign {
    Q_GADGET
    QML_NAMED_ELEMENT(AmenitySortFilterProxyModel)
    QML_FOREIGN(KOSMIndoorMap::AmenitySortFilterProxyModel)
};

struct FloorLevelChangeModelForeign {
    Q_GADGET
    QML_NAMED_ELEMENT(FloorLevelChangeModel)
    QML_FOREIGN(KOSMIndoorMap::FloorLevelChangeModel)
};

struct MapItemForeign {
    Q_GADGET
    QML_NAMED_ELEMENT(MapItemImpl)
    QML_FOREIGN(KOSMIndoorMap::MapItem)
};

struct OSMElementInformationModelForeign {
    Q_GADGET
    QML_NAMED_ELEMENT(OSMElementInformationModel)
    QML_FOREIGN(KOSMIndoorMap::OSMElementInformationModel)
};

struct RoomModelForeign {
    Q_GADGET
    QML_NAMED_ELEMENT(RoomModel)
    QML_FOREIGN(KOSMIndoorMap::RoomModel)
};

struct RoomSortFilterProxyModelForeign {
    Q_GADGET
    QML_NAMED_ELEMENT(RoomSortFilterProxyModel)
    QML_FOREIGN(KOSMIndoorMap::RoomSortFilterProxyModel)
};

#endif
