/*
    SPDX-FileCopyrightText: 2024 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KOSMINDOORROUTING_RECASTSETTINGS_P_H
#define KOSMINDOORROUTING_RECASTSETTINGS_P_H

namespace KOSMIndoorRouting {

constexpr inline float RECAST_CELL_SIZE = 0.2f;
constexpr inline float RECAST_CELL_HEIGHT = 0.2f;

constexpr inline float RECAST_AGENT_HEIGHT = 2.0f;
constexpr inline float RECAST_AGENT_RADIUS = 0.3f;
constexpr inline float RECAST_AGENT_MAX_CLIMB = 0.9f;
constexpr inline float RECAST_AGENT_MAX_SLOPE = 75.0f;

enum class RecastPartitionType {
    Watershed,
    Monotone,
    Layers,
};

constexpr inline RecastPartitionType RECAST_PARTITION_TYPE = RecastPartitionType::Watershed;

}

#endif
