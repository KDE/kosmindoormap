/*
    SPDX-FileCopyrightText: 2024 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "routingcontroller.h"

#include "navmeshbuilder.h"
#include "routingjob.h"

#include <QDebug>

using namespace KOSMIndoorRouting;

RoutingController::RoutingController(QObject *parent)
    : QObject(parent)
{
}

RoutingController::~RoutingController() = default;

void RoutingController::setMapData(const KOSMIndoorMap::MapData &mapData)
{
    if (m_mapData == mapData) {
        return;
    }
    m_mapData = mapData;
    m_navMesh.clear();

    Q_EMIT mapDataChanged();
}

// TODO nav mesh rebuild when elevator model changes

void RoutingController::setStartPosition(double lat, double lon, int floorLevel)
{
    qDebug() << lat << lon <<floorLevel;
    m_start = OSM::Coordinate{lat, lon};
    m_startLevel = floorLevel;
}

void RoutingController::setEndPosition(double lat, double lon, int floorLevel)
{
    qDebug() << lat << lon <<floorLevel;
    m_end = OSM::Coordinate{lat, lon};
    m_endLevel = floorLevel;
}

void RoutingController::searchRoute()
{
    // TODO state tracking, protect against double runs
    if (m_navMesh.isNull()) {
        buildNavMesh();
        return;
    }

    auto router = new RoutingJob(this);
    router->setNavMesh(m_navMesh);
    router->setStart(m_navMesh.transform().mapGeoHeightToNav(m_start, m_startLevel));
    router->setEnd(m_navMesh.transform().mapGeoHeightToNav(m_end, m_endLevel));
    connect(router, &RoutingJob::finished, this, [this, router]() {
        router->deleteLater();
        // TODO
    });
    router->start();

}

void RoutingController::buildNavMesh()
{
    auto builder = new NavMeshBuilder(this);
    builder->setMapData(m_mapData);
    builder->setEquipmentModel(m_elevatorModel);
    connect(builder, &NavMeshBuilder::finished, this, [this, builder]() {
        builder->deleteLater();
        m_navMesh = builder->navMesh();
        // TODO loop protection/error handling
        searchRoute();
    });
    builder->start();
}

#include "moc_routingcontroller.cpp"
