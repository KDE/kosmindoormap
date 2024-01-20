/*
    SPDX-FileCopyrightText: 2024 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KOSMINDOORROUTING_NAVMESH_P_H
#define KOSMINDOORROUTING_NAVMESH_P_H

#include "navmesh.h"
#include "navmeshtransform.h"
#include "recastnav_p.h"

namespace KOSMIndoorRouting {
class NavMeshPrivate
{
public:
    [[nodiscard]] static inline NavMeshPrivate *get(const NavMesh &navMesh) {
        return navMesh.d.get();
    }

    [[nodiscard]] static inline NavMeshPrivate *create(NavMesh &navMesh) {
        assert(!navMesh.d);
        navMesh.d = std::make_shared<NavMeshPrivate>();
        return navMesh.d.get();
    }

#if HAVE_RECAST
    dtNavMeshPtr m_navMesh;
    dtNavMeshQueryPtr m_navMeshQuery;
#endif

    NavMeshTransform m_transform;
};
}

#endif
