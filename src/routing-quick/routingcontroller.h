/*
    SPDX-FileCopyrightText: 2024 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KOSMINDOORROUTING_ROUTINGCONTROLLER_H
#define KOSMINDOORROUTING_ROUTINGCONTROLLER_H

#include "navmesh.h"

#include <KOSMIndoorMap/OverlaySource>
#include <KOSMIndoorMap/MapData>

#include <KOSMIndoorRouting/Route>

#include <qqmlregistration.h>
#include <QObject>

namespace KOSMIndoorRouting {

class RouteOverlay;

/** Routing interface for QML. */
class RoutingController : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(KOSMIndoorMap::MapData mapData MEMBER m_mapData WRITE setMapData NOTIFY mapDataChanged)
    Q_PROPERTY(KOSMIndoorMap::AbstractOverlaySource *elevatorModel MEMBER m_elevatorModel NOTIFY elevatorModelChanged)
    Q_PROPERTY(KOSMIndoorMap::AbstractOverlaySource *routeOverlay READ routeOverlay CONSTANT)
    // TODO routing profile
public:
    explicit RoutingController(QObject *parent = nullptr);
    ~RoutingController();

    Q_INVOKABLE void setStartPosition(double lat, double lon, int floorLevel);
    Q_INVOKABLE void setEndPosition(double lat, double lon, int floorLevel);

    [[nodiscard]] KOSMIndoorMap::AbstractOverlaySource* routeOverlay() const;

public Q_SLOTS:
    void searchRoute();

Q_SIGNALS:
    void mapDataChanged();
    void elevatorModelChanged();

private:
    void setMapData(const KOSMIndoorMap::MapData &mapData);

    void buildNavMesh();

    KOSMIndoorMap::MapData m_mapData;
    KOSMIndoorMap::AbstractOverlaySource *m_elevatorModel = nullptr;
    NavMesh m_navMesh;
    Route m_route;

    OSM::Coordinate m_start;
    OSM::Coordinate m_end;
    int m_startLevel = 0;
    int m_endLevel = 0;

    RouteOverlay *m_routeOverlay = nullptr;
};
}

#endif
