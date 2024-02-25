/*
    SPDX-FileCopyrightText: 2024 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KOSMINDOORROUTING_RECASTNAVSETTINGS_P_H
#define KOSMINDOORROUTING_RECASTNAVSETTINGS_P_H

namespace KOSMIndoorRouting {

constexpr inline float RECAST_CELL_SIZE = 0.2f;
constexpr inline float RECAST_CELL_HEIGHT = 0.2f;

constexpr inline float RECAST_AGENT_HEIGHT = 2.0f;
constexpr inline float RECAST_AGENT_RADIUS = 0.2f;
constexpr inline float RECAST_AGENT_MAX_CLIMB = 0.9f;
constexpr inline float RECAST_AGENT_MAX_SLOPE = 75.0f;

constexpr inline int RECAST_REGION_MIN_AREA = 8;
constexpr inline int RECAST_REGION_MERGE_AREA = 20;

constexpr inline float RECAST_MAX_EDGE_LEN = 12.0f;
constexpr inline float RECAST_MAX_SIMPLIFICATION_ERROR = 1.3f;

constexpr inline float RECAST_DETAIL_SAMPLE_DIST = 6.0f;
constexpr inline float RECAST_DETAIL_SAMPLE_MAX_ERROR = 1.0f;

enum class RecastPartitionType {
    Watershed,
    Monotone,
    Layers,
};

constexpr inline RecastPartitionType RECAST_PARTITION_TYPE = RecastPartitionType::Watershed;

// if this is too small findPath() will returns DT_OUT_OF_NODES
constexpr inline int  RECAST_NAV_QUERY_MAX_NODES = 8192;

}

#endif
