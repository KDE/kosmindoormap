/*
    SPDX-FileCopyrightText: 2024 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "routingjob.h"

#include <logging.h>
#include "navmesh.h"
#include "navmesh_p.h"
#include "navmeshtransform.h"
#include "route.h"
#include "routingarea.h"
#include "routingprofile.h"

#include <QThreadPool>

namespace KOSMIndoorRouting {
class RoutingJobPrivate {
public:
    void performQuery();

    NavMesh m_navMesh;
    rcVec3 m_start;
    rcVec3 m_end;
    RoutingProfile m_profile;
    Route m_route;
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

void RoutingJob::setRoutingProfile(const RoutingProfile &profile)
{
    d->m_profile = profile;
}

void RoutingJob::start()
{
    qCDebug(Log) << QThread::currentThread();

    QThreadPool::globalInstance()->start([this]() {
        d->performQuery();
        QMetaObject::invokeMethod(this, &RoutingJob::finished, Qt::QueuedConnection);
    });
}

Route RoutingJob::route() const
{
    return d->m_route;
}

void RoutingJobPrivate::performQuery()
{
    qCDebug(Log) << QThread::currentThread();

    const auto navMesh = NavMeshPrivate::get(m_navMesh);
    qCDebug(Log) <<m_start.x <<m_start.y << m_start.z << m_end.x << m_end.y << m_end.z;
#if HAVE_RECAST
    dtQueryFilter filter;
    filter.setIncludeFlags(m_profile.flags());
    filter.setExcludeFlags(~m_profile.flags());
    for (int i = 1; i <AREA_TYPE_COUNT - 1; ++i) {
        filter.setAreaCost(i, m_profile.cost(static_cast<AreaType>(i)));
    }
    filter.setAreaCost(RC_WALKABLE_AREA, m_profile.cost(AreaType::Walkable));
    qCDebug(Log) << filter.getIncludeFlags() << filter.getExcludeFlags();

    rcVec3 polyPickExt({ 2.0f, 4.0f, 2.0f }); // ???
    dtPolyRef startPoly;
    navMesh->m_navMeshQuery->findNearestPoly(m_start, polyPickExt, &filter, &startPoly, nullptr);
    dtPolyRef endPoly;
    navMesh->m_navMeshQuery->findNearestPoly(m_end, polyPickExt, &filter, &endPoly, nullptr);

    dtPolyRef path[256];
    int pathCount = 0;
    qCDebug(Log) <<startPoly <<endPoly;
    auto status = navMesh->m_navMeshQuery->findPath(startPoly, endPoly, m_start, m_end, &filter, path, &pathCount, 256); // TODO
    qCDebug(Log) << pathCount << status;

    std::vector<rcVec3> straightPath;
    straightPath.resize(256);
    std::vector<uint8_t> straightPathFlags;
    straightPathFlags.resize(256);
    int straightPathCount = 0;
    dtPolyRef straightPathPolys[256];
    status = navMesh->m_navMeshQuery->findStraightPath(m_start, m_end, path, pathCount, (float*)straightPath.data(), straightPathFlags.data(), straightPathPolys, &straightPathCount, 256, 0);
    qCDebug(Log) <<straightPathCount << status;
    std::vector<RouteStep> steps;
    steps.reserve(straightPathCount);
    for (int i = 0; i <straightPathCount; ++i) {
        qCDebug(Log) << "  " << straightPath[i].x << " " <<straightPath[i].y << " " << straightPath[i].z << " " <<straightPathFlags[i];
        steps.push_back({m_navMesh.transform().mapNavToGeo(straightPath[i]), m_navMesh.transform().mapNavHeightToFloorLevel(straightPath[i].y)});
    }
    m_route.setSteps(std::move(steps));
#endif
}

#include "moc_routingjob.cpp"
