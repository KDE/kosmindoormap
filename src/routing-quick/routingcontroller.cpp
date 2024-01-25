/*
    SPDX-FileCopyrightText: 2024 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "routingcontroller.h"

#include "navmeshbuilder.h"
#include "routingjob.h"
#include "routeoverlay.h"

#include <QDebug>

using namespace KOSMIndoorRouting;

RoutingController::RoutingController(QObject *parent)
    : QObject(parent)
    , m_routeOverlay(new RouteOverlay(this))
{
}

RoutingController::~RoutingController() = default;

bool RoutingController::routingAvailable() const
{
#if HAVE_RECAST
    return true;
#else
    return false;
#endif
}

bool RoutingController::routingInProgress() const
{
    return m_builder || m_routingJob;
}

void RoutingController::setMapData(const KOSMIndoorMap::MapData &mapData)
{
    if (m_mapData == mapData) {
        return;
    }
    m_mapData = mapData;
    m_navMesh.clear();
    m_routeOverlay->setMapData(mapData);

    // TODO cancel ongoing jobs

    Q_EMIT mapDataChanged();
}

// TODO nav mesh rebuild when elevator model changes

void RoutingController::setStartPosition(double lat, double lon, int floorLevel)
{
    qDebug() << lat << lon <<floorLevel;
    m_start = OSM::Coordinate{lat, lon};
    m_startLevel = floorLevel;
    m_routeOverlay->setStart(m_start, m_startLevel);
}

void RoutingController::setEndPosition(double lat, double lon, int floorLevel)
{
    qDebug() << lat << lon <<floorLevel;
    m_end = OSM::Coordinate{lat, lon};
    m_endLevel = floorLevel;
    m_routeOverlay->setEnd(m_end, m_endLevel);
}

KOSMIndoorMap::AbstractOverlaySource* RoutingController::routeOverlay() const
{
    return m_routeOverlay;
}

void RoutingController::searchRoute()
{
    if (m_builder) { // already running
        return;
    }

    if (m_navMesh.isNull()) {
        buildNavMesh();
        Q_EMIT progressChanged();
        return;
    }

    auto router = new RoutingJob(this);
    router->setNavMesh(m_navMesh);
    router->setStart(m_navMesh.transform().mapGeoHeightToNav(m_start, m_startLevel));
    router->setEnd(m_navMesh.transform().mapGeoHeightToNav(m_end, m_endLevel));
    connect(router, &RoutingJob::finished, this, [this, router]() {
        router->deleteLater();
        if (m_routingJob == router) {
            m_routeOverlay->setRoute(router->route());
            m_routingJob = nullptr;
        }
        Q_EMIT progressChanged();
    });
    m_routingJob = router;
    router->start();
    Q_EMIT progressChanged();
}

void RoutingController::buildNavMesh()
{
    auto builder = new NavMeshBuilder(this);
    builder->setMapData(m_mapData);
    builder->setEquipmentModel(m_elevatorModel);
    connect(builder, &NavMeshBuilder::finished, this, [this, builder]() {
        builder->deleteLater();
        if (m_builder == builder) {
            m_navMesh = builder->navMesh();
            m_builder = nullptr;
        }
        // TODO loop protection/error handling
        searchRoute();
    });
    builder->start();
    m_builder = builder;
}

#include "moc_routingcontroller.cpp"
