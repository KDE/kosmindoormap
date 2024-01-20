/*
    SPDX-FileCopyrightText: 2024 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "routingjob.h"

#include <logging.h>
#include "navmesh.h"
#include "navmesh_p.h"
#include "navmeshtransform.h"

#include <QThreadPool>

namespace KOSMIndoorRouting {
class RoutingJobPrivate {
public:
    void performQuery();

    NavMesh m_navMesh;
    rcVec3 m_start;
    rcVec3 m_end;
};
}

using namespace KOSMIndoorRouting;

RoutingJob::RoutingJob(QObject *parent)
    : QObject(parent)
    , d(std::make_unique<RoutingJobPrivate>())
{
}

RoutingJob::~RoutingJob() = default;

void RoutingJob::setNavMesh(const NavMesh &navMesh)
{
    d->m_navMesh = navMesh;
}

void RoutingJob::setStart(rcVec3 start)
{
    d->m_start = start;
}

void RoutingJob::setEnd(rcVec3 end)
{
    d->m_end = end;
}

void RoutingJob::start()
{
    qCDebug(Log) << QThread::currentThread();

    QThreadPool::globalInstance()->start([this]() {
        d->performQuery();
        QMetaObject::invokeMethod(this, &RoutingJob::finished, Qt::QueuedConnection);
    });
}

void RoutingJobPrivate::performQuery()
{
    qCDebug(Log) << QThread::currentThread();

    const auto navMesh = NavMeshPrivate::get(m_navMesh);
    qCDebug(Log) <<m_start.x <<m_start.y << m_start.z << m_end.x << m_end.y << m_end.z;
#if HAVE_RECAST
    dtQueryFilter filter; // TODO
    filter.setIncludeFlags(0xffef);
    filter.setExcludeFlags(0);
    for (int i = 0; i < 8; ++i) {
        filter.setAreaCost(i, 1.0f);
    }
    qCDebug(Log) << filter.getIncludeFlags() << filter.getExcludeFlags();

    rcVec3 polyPickExt({ 2.0f, 4.0f, 2.0f }); // ???
    dtPolyRef startPoly;
    navMesh->m_navMeshQuery->findNearestPoly(m_start, polyPickExt, &filter, &startPoly, nullptr);
    dtPolyRef endPoly;
    navMesh->m_navMeshQuery->findNearestPoly(m_end, polyPickExt, &filter, &endPoly, nullptr);

    dtPolyRef path[256];
    int pathCount = 0;
    qCDebug(Log) <<startPoly <<endPoly;
    navMesh->m_navMeshQuery->findPath(startPoly, endPoly, m_start, m_end, &filter, path, &pathCount, 256); // TODO
    qCDebug(Log) << pathCount;

    std::vector<rcVec3> straightPath;
    straightPath.resize(256);
    std::vector<uint8_t> straightPathFlags;
    straightPathFlags.resize(256);
    int straightPathCount = 0;
    dtPolyRef straightPathPolys[256];
    navMesh->m_navMeshQuery->findStraightPath(m_start, m_end, path, pathCount, (float*)straightPath.data(), straightPathFlags.data(), straightPathPolys, &straightPathCount, 256, 0);
    qCDebug(Log) <<straightPathCount;
    for (int i = 0; i <straightPathCount; ++i) {
        qCDebug(Log) << "  " << straightPath[i].x << " " <<straightPath[i].y << " " << straightPath[i].z << " " <<straightPathFlags[i];
    }
#endif
}

#include "moc_routingjob.cpp"
