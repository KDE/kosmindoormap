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

#define FOREIGN_ENUM_GADGET(Class) \
class Class ## Derived: public KOSMIndoorMap::Class \
{ \
    Q_GADGET \
}; \
namespace Class ## DerivedForeign \
{ \
    Q_NAMESPACE \
    QML_NAMED_ELEMENT(Class) \
    QML_FOREIGN_NAMESPACE(Class ## Derived) \
}


// from base library
struct EquipmentModelForeign {
    Q_GADGET
    QML_NAMED_ELEMENT(EquipmentModel)
    QML_FOREIGN(KOSMIndoorMap::EquipmentModel)
};

struct FloorLevelModelForeign {
    Q_GADGET
    QML_NAMED_ELEMENT(FloorLevelModel)
    QML_FOREIGN(KOSMIndoorMap::FloorLevelModel)
    QML_UNCREATABLE("only provided via C++ API")
};

struct GateModelForeign {
    Q_GADGET
    QML_NAMED_ELEMENT(GateModel)
    QML_FOREIGN(KOSMIndoorMap::GateModel)
};

struct MapDataForeign {
    Q_GADGET
    QML_FOREIGN(KOSMIndoorMap::MapData)
    QML_VALUE_TYPE(mapData)
    QML_UNCREATABLE("only provided via C++ API")
};

struct MapLoaderForeign {
    Q_GADGET
    QML_NAMED_ELEMENT(MapLoader)
    QML_FOREIGN(KOSMIndoorMap::MapLoader)
    QML_UNCREATABLE("only provided via C++ API")
};

struct PlatformForeign {
    Q_GADGET
    QML_VALUE_TYPE(platform)
    QML_FOREIGN(KOSMIndoorMap::Platform)
};
FOREIGN_ENUM_GADGET(Platform)

struct PlatformModelForeign {
    Q_GADGET
    QML_NAMED_ELEMENT(PlatformModel)
    QML_FOREIGN(KOSMIndoorMap::PlatformModel)
};

struct ViewForeign {
    Q_GADGET
    QML_NAMED_ELEMENT(View)
    QML_FOREIGN(KOSMIndoorMap::View)
    QML_UNCREATABLE("only provided via C++ API")
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

struct OSMElementForeign {
    Q_GADGET
    QML_NAMED_ELEMENT(osmElement)
    QML_FOREIGN(KOSMIndoorMap::OSMElement)
    QML_UNCREATABLE("only provided via C++ API")
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
