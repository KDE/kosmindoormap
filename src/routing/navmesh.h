/*
    SPDX-FileCopyrightText: 2024 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KOSMINDOORROUTING_NAVMESH_H
#define KOSMINDOORROUTING_NAVMESH_H

#include "kosmindoorrouting_export.h"

#include <memory>

namespace KOSMIndoorRouting {

class NavMeshTransform;
class NavMeshPrivate;

/** Compiled nav mesh for routing */
class KOSMINDOORROUTING_EXPORT NavMesh
{
public:
    explicit NavMesh();
    NavMesh(NavMesh &&) noexcept;
    NavMesh(const NavMesh&);
    ~NavMesh();
    NavMesh& operator=(NavMesh&&) noexcept;
    NavMesh& operator=(const NavMesh&);

    [[nodiscard]] bool isNull() const;
    void clear();

    [[nodiscard]] NavMeshTransform transform() const;

private:
    friend class NavMeshPrivate;
    std::shared_ptr<NavMeshPrivate> d;
};
}

#endif
