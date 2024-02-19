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
    m_start = {};
    m_end = {};

    // TODO cancel ongoing jobs
    m_builder = nullptr;
    m_routingJob = nullptr;

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

void RoutingController::setProfile(const RoutingProfile &profile)
{
    qDebug() << profile.flags();
    if (profile == m_routingProfile) {
        return;
    }
    m_routingProfile = profile;
    Q_EMIT profileChanged();
}

void RoutingController::searchRoute()
{
    qDebug();
    if (m_builder) { // already running
        return;
    }

    if (!m_start.isValid() && !m_end.isValid()) {
        return;
    }

    if (!m_navMesh.isValid()) {
        buildNavMesh();
        Q_EMIT progressChanged();
        return;
    }

    auto router = new RoutingJob(this);
    router->setNavMesh(m_navMesh);
    router->setStart(m_navMesh.transform().mapGeoHeightToNav(m_start, m_startLevel));
    router->setEnd(m_navMesh.transform().mapGeoHeightToNav(m_end, m_endLevel));
    router->setRoutingProfile(m_routingProfile);
    connect(router, &RoutingJob::finished, this, [this, router]() {
        router->deleteLater();
        if (m_routingJob == router) {
            m_routeOverlay->setRoute(router->route());
            m_routingJob = nullptr;

            // navmesh became invalid during routing...
            if (!m_navMesh.isValid()) {
                QMetaObject::invokeMethod(this, &RoutingController::searchRoute);
            }
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
        if (m_navMesh.isValid()) {
            searchRoute();
        } else {
            qWarning() << "Failed to generate nav mesh, routing not available!";
        }

        Q_EMIT progressChanged();
    });
    builder->start();
    m_builder = builder;
}

#include "moc_routingcontroller.cpp"
