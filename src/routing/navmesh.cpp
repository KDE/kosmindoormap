/*
    SPDX-FileCopyrightText: 2024 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "navmesh.h"
#include "navmesh_p.h"

using namespace KOSMIndoorRouting;

NavMesh::NavMesh() = default;
NavMesh::NavMesh(NavMesh&&) noexcept = default;
NavMesh::NavMesh(const NavMesh&) = default;
NavMesh::~NavMesh() = default;
NavMesh& NavMesh::operator=(NavMesh&&) noexcept = default;
NavMesh& NavMesh::operator=(const NavMesh&) = default;

bool NavMesh::isNull() const
{
    return d == nullptr;
}

void NavMesh::clear()
{
    d.reset();
}

NavMeshTransform NavMesh::transform() const
{
    return d ? d->m_transform : NavMeshTransform();
}
