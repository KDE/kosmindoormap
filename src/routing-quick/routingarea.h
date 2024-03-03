/*
    SPDX-FileCopyrightText: 2024 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <KOSMIndoorRouting/RoutingArea>

#include <QQmlEngine>
#include <QQmlExtensionPlugin>

namespace AreaType {
Q_NAMESPACE
QML_NAMED_ELEMENT(AreaType)

#define E(x) x = qToUnderlying(KOSMIndoorRouting::AreaType::x)

// ugly, but QML can't do enum classes and/or enums != 32 bit
enum AreaType {
    E(Unwalkable),
    E(Stairs),
    E(Elevator),
    E(Escalator),
    E(MovingWalkway),
    E(TactilePaving),
    E(StreetCrossing),
    E(Ramp),
    E(Room),
    E(Walkable),
};

#undef E

Q_ENUM_NS(AreaType)
}

namespace AreaFlag {
Q_NAMESPACE
QML_NAMED_ELEMENT(AreaFlag)

#define E(x) x = qToUnderlying(KOSMIndoorRouting::AreaFlag::x)

enum AreaFlag {
    E(NoFlag),
    E(Walkable),
    E(Stairs),
    E(Escalator),
    E(Elevator),
};

#undef E

Q_ENUM_NS(AreaFlag)

}
