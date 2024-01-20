/*
    SPDX-FileCopyrightText: 2024 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KOSMINDOORROUTING_RECASTNAV_P_H
#define KOSMINDOORROUTING_RECASTNAV_P_H

#include <functional>
#include <memory>

#if HAVE_RECAST

#include <DetourNavMesh.h>
#include <DetourNavMeshQuery.h>
#include <Recast.h>

namespace KOSMIndoorRouting {

namespace detail {
    template <typename T, void(*F)(T*)>
    struct deleter {
        void operator()(T *ptr) { std::invoke(F, ptr); }
    };
}

using dtNavMeshPtr = std::unique_ptr<dtNavMesh, detail::deleter<dtNavMesh, &dtFreeNavMesh>>;
using dtNavMeshQueryPtr = std::unique_ptr<dtNavMeshQuery, detail::deleter<dtNavMeshQuery, &dtFreeNavMeshQuery>>;

using rcCompactHeightfieldPtr = std::unique_ptr<rcCompactHeightfield, detail::deleter<rcCompactHeightfield, &rcFreeCompactHeightfield>>;
using rcContourSetPtr = std::unique_ptr<rcContourSet, detail::deleter<rcContourSet, &rcFreeContourSet>>;
using rcHeightfieldPtr = std::unique_ptr<rcHeightfield, detail::deleter<rcHeightfield, &rcFreeHeightField>>;
using rcPolyMeshPtr = std::unique_ptr<rcPolyMesh, detail::deleter<rcPolyMesh, &rcFreePolyMesh>>;
using rcPolyMeshDetailPtr = std::unique_ptr<rcPolyMeshDetail, detail::deleter<rcPolyMeshDetail, &rcFreePolyMeshDetail>>;
using rcAllocPolyMeshDetailPtr = std::unique_ptr<rcPolyMeshDetail, detail::deleter<rcPolyMeshDetail, &rcFreePolyMeshDetail>>;

}

#endif

#endif
