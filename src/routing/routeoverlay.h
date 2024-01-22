/*
    SPDX-FileCopyrightText: 2024 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KOSMINDOORROUTING_ROUTEOVERLAY_H
#define KOSMINDOORROUTING_ROUTEOVERLAY_H

#include "kosmindoorrouting_export.h"
#include "route.h"

#include <KOSMIndoorMap/MapData>
#include <KOSMIndoorMap/OverlaySource>

namespace KOSMIndoorRouting {

class KOSMINDOORROUTING_EXPORT RouteOverlay : public KOSMIndoorMap::AbstractOverlaySource
{
    Q_OBJECT
public:
    explicit RouteOverlay(QObject *parent = nullptr);
    ~RouteOverlay();

    void setMapData(const KOSMIndoorMap::MapData &mapData);

    void setStart(OSM::Coordinate c, int level);
    void setEnd(OSM::Coordinate c, int level);
    void setRoute(const Route &route);

    void forEach(int floorLevel, const std::function<void(OSM::Element, int)> &func) const override;
    void endSwap() override;
    [[nodiscard]] const std::vector<OSM::Node>* transientNodes() const override;

private:
    KOSMIndoorMap::MapData m_data;
    OSM::TagKey m_mxRouteKey;

    OSM::UniqueElement m_startNode;
    OSM::UniqueElement m_endNode;
    std::vector<OSM::UniqueElement> m_routeWays;
    std::vector<int> m_routeWayFloorLevels;
    std::vector<OSM::Node> m_transientNodes;

    int m_startLevel = 0;
    int m_endLevel = 0;
    Route m_route;

    std::vector<OSM::UniqueElement> m_gc;
    std::vector<std::vector<OSM::Node>> m_transientNodesGC;
};

}

#endif
