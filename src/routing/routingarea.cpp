/*
    SPDX-FileCopyrightText: 2024 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "routingarea.h"
#include "logging.h"

#include <algorithm>

using namespace KOSMIndoorRouting;

// ### keep sorted by area type
struct {
    AreaType area;
    AreaFlags flags;
} constexpr inline const area_type_flag_map[] = {
    { AreaType::Unwalkable, {} },
    { AreaType::Stairs, AreaFlag::Stairs },
    { AreaType::Elevator, AreaFlag::Elevator },
    { AreaType::Escalator, AreaFlag::Escalator },
    { AreaType::MovingWalkway, AreaFlag::Escalator },
    { AreaType::TactilePaving, AreaFlag::Walkable },
    { AreaType::StreetCrossing, AreaFlag::Walkable },
    { AreaType::Ramp, AreaFlag::Walkable },
    { AreaType::Walkable, AreaFlag::Walkable },
};

AreaFlags KOSMIndoorRouting::flagsForAreaType(AreaType area)
{
    const auto it = std::lower_bound(std::begin(area_type_flag_map), std::end(area_type_flag_map), area, [](auto lhs, auto rhs) {
        return lhs.area < rhs;
    });
    if (it != std::end(area_type_flag_map) && (*it).area == area) {
        return (*it).flags;
    }

    qCWarning(Log) << "no area flags for area type defined:" << qToUnderlying(area);
    return {};
}

