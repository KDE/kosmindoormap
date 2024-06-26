/*
    SPDX-FileCopyrightText: 2024 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KOSMINDOORROUTING_NAVMESH_H
#define KOSMINDOORROUTING_NAVMESH_H

#include "kosmindoorrouting_export.h"

#include <memory>

class QString;

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

    /** Returns @c true if the nav mesh is neither null (ie. not built)
     *  and hasn't been invalidated by realtime data changes.
     *  The latter implies that this can change on existing instances.
     */
    [[nodiscard]] bool isValid() const;

    void clear();

    [[nodiscard]] NavMeshTransform transform() const;

    /** Write nav mesh data to the given file.
     *  Uses the file format used by the Recast demo, so this is primarily
     *  for debugging.
     */
    void writeToFile(const QString &fileName) const;

private:
    friend class NavMeshPrivate;
    std::shared_ptr<NavMeshPrivate> d;
};
}

#endif
