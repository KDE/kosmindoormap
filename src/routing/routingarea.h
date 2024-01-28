/*
    SPDX-FileCopyrightText: 2024 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KOSMINDOORROUTING_ROUTINGAREA_H
#define KOSMINDOORROUTING_ROUTINGAREA_H

#include <QFlags>

#include <cstdint>

namespace KOSMIndoorRouting {

/** Area types used for the routing engine
 *  This can be a maximum of 64 different values, with 0 and 63 having pre-defined meanings.
 *  Each area type can have a separate cost factor and routing flag assigned.
 */
enum class AreaType : uint8_t {
    Unwalkable = 0, // RC_NULL_AREA
    Stairs,
    Elevator,
    Escalator,
    MovingWalkway,
    TactilePaving,
    StreetCrossing,
    Ramp,
    Walkable = 63, // RC_WALKABLE_AREA
};

constexpr inline int AREA_TYPE_COUNT = 9;

/** Area flags used for routing profiles.
 *  There are 16 possible values, mapped to from AreaType.
 */
enum class AreaFlag : uint16_t {
    NoFlag = 0,
    Walkable = 1,
    Stairs = 2,
    Escalator = 4,
    Elevator = 8,
};

Q_DECLARE_FLAGS(AreaFlags, AreaFlag)

/** Flags for an area type. */
[[nodiscard]] AreaFlags flagsForAreaType(AreaType area);

}

Q_DECLARE_OPERATORS_FOR_FLAGS(KOSMIndoorRouting::AreaFlags)

#endif
