/*
    SPDX-FileCopyrightText: 2024 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "navmesh.h"
#include "navmesh_p.h"
#include "logging.h"

#include <QFile>

#include <cstdint>

using namespace KOSMIndoorRouting;

#if HAVE_RECAST
constexpr inline const uint32_t NAVMESHSET_MAGIC = 'M' << 24 | 'S' << 16 | 'E' << 8 | 'T';
constexpr inline const uint32_t NAVMESHSET_VERSION = 1;

struct NavMeshSetHeader
{
    uint32_t magic = NAVMESHSET_MAGIC;
    uint32_t version = NAVMESHSET_VERSION;
    uint32_t numTiles = 0;
    dtNavMeshParams params;
};

struct NavMeshTileHeader
{
    dtTileRef tileRef;
    uint32_t dataSize;
};
#endif


NavMesh::NavMesh() = default;
NavMesh::NavMesh(NavMesh&&) noexcept = default;
NavMesh::NavMesh(const NavMesh&) = default;
NavMesh::~NavMesh() = default;
NavMesh& NavMesh::operator=(NavMesh&&) noexcept = default;
NavMesh& NavMesh::operator=(const NavMesh&) = default;

bool NavMesh::isValid() const
{
    return d && !d->m_dirty;
}

void NavMesh::clear()
{
    d.reset();
}

NavMeshTransform NavMesh::transform() const
{
    return d ? d->m_transform : NavMeshTransform();
}

void NavMesh::writeToFile(const QString &fileName) const
{
    QFile f(fileName);
    if (!f.open(QFile::WriteOnly)) {
        qCWarning(Log) << fileName << f.errorString();
        return;
    }
#if HAVE_RECAST
    const auto *mesh = d->m_navMesh.get();

    NavMeshSetHeader header;
    for (auto i = 0; i < mesh->getMaxTiles(); ++i) {
        const auto tile = mesh->getTile(i);
        if (!tile || !tile->header || !tile->dataSize) {
            continue;
        }
        header.numTiles++;
    }
    std::memcpy(&header.params, d->m_navMesh->getParams(), sizeof(dtNavMeshParams));
    f.write(reinterpret_cast<const char*>(&header), sizeof(NavMeshSetHeader));

    for (auto i = 0; i < mesh->getMaxTiles(); ++i) {
        const auto tile = mesh->getTile(i);
        if (!tile || !tile->header || !tile->dataSize) {
            continue;
        }

        NavMeshTileHeader tileHeader;
        tileHeader.tileRef = mesh->getTileRef(tile);
        tileHeader.dataSize = tile->dataSize;

        f.write(reinterpret_cast<const char*>(&tileHeader), sizeof(NavMeshTileHeader));
        f.write(reinterpret_cast<const char*>(tile->data), tile->dataSize);
    }
#endif
}
