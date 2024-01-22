/*
    SPDX-FileCopyrightText: 2024 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "routeoverlay.h"

using namespace KOSMIndoorRouting;

RouteOverlay::RouteOverlay(QObject *parent)
    : KOSMIndoorMap::AbstractOverlaySource(parent)
{
}

RouteOverlay::~RouteOverlay() = default;

void RouteOverlay::setMapData(const KOSMIndoorMap::MapData &mapData)
{
    m_data = mapData;
    if (mapData.isEmpty()) {
        return;
    }

    m_mxRouteKey = m_data.dataSet().makeTagKey("mx:routing");
    Q_EMIT reset();
}

void RouteOverlay::setStart(OSM::Coordinate c, int floorLevel)
{
    if (m_startNode) {
        m_gc.push_back(std::move(m_startNode));
    }
    if (c.isValid()) {
        m_startNode = OSM::UniqueElement(new OSM::Node);
        m_startNode.setId(m_data.dataSet().nextInternalId());
        m_startNode.node()->coordinate = c;
        m_startNode.setTagValue(m_mxRouteKey, "start");
        m_startLevel = floorLevel;
    }
    Q_EMIT update();
}

void RouteOverlay::setEnd(OSM::Coordinate c, int floorLevel)
{
    if (m_endNode) {
        m_gc.push_back(std::move(m_endNode));
    }
    if (c.isValid()) {
        m_endNode = OSM::UniqueElement(new OSM::Node);
        m_endNode.setId(m_data.dataSet().nextInternalId());
        m_endNode.node()->coordinate = c;
        m_endNode.setTagValue(m_mxRouteKey, "end");
        m_endLevel = floorLevel;
    }
    Q_EMIT update();
}

void RouteOverlay::setRoute(const Route &route)
{
    for (auto &way : m_routeWays) {
        m_gc.push_back(std::move(way));
    }
    m_routeWays.clear();
    m_routeWayFloorLevels.clear();
    m_transientNodesGC.push_back(std::move(m_transientNodes));

    m_route = route;
    if (m_route.steps().size() < 2) {
        Q_EMIT update();
        return;
    }

    m_transientNodes.reserve(m_route.steps().size());
    OSM::BoundingBox bbox;

    int prevLevel = m_route.steps()[0].floorLevel;

    OSM::UniqueElement way;
    for (auto it = m_route.steps().begin(); it != m_route.steps().end();) {
        // TODO compute per segment rather than globally
        bbox = OSM::unite(bbox, {(*it).coordinate, (*it).coordinate});

        if (!way) {
            way = OSM::UniqueElement(new OSM::Way);
            way.setId(m_data.dataSet().nextInternalId());
            way.setTagValue(m_mxRouteKey, "route");
        }

        OSM::Node node;
        node.id = m_data.dataSet().nextInternalId();
        node.coordinate = (*it).coordinate;
        way.way()->nodes.push_back(node.id);
        m_transientNodes.push_back(std::move(node));

        if (way.way()->nodes.size() >= 2 && (*it).floorLevel != prevLevel) {
            m_routeWays.push_back(std::move(way));
            m_routeWayFloorLevels.push_back(prevLevel);
            prevLevel = (*it).floorLevel;
            --it;
        } else {
            ++it;
        }
    }

    if (way) {
        m_routeWays.push_back(std::move(way));
        m_routeWayFloorLevels.push_back(prevLevel);
    }

    std::sort(m_transientNodes.begin(), m_transientNodes.end());
    for (auto &way :m_routeWays) {
        way.way()->bbox = bbox;
    }

    Q_EMIT update();
}

void RouteOverlay::forEach(int floorLevel, const std::function<void(OSM::Element, int)> &func) const
{
    for (std::size_t i = 0; i < m_routeWays.size(); ++i) {
        if (floorLevel == m_routeWayFloorLevels[i]) {
            func(m_routeWays[i], floorLevel);
        }
    }

    if (m_startNode && m_startLevel == floorLevel) {
        func(m_startNode, floorLevel);
    }
    if (m_endNode && m_endLevel == floorLevel) {
        func(m_endNode, floorLevel);
    }
}

void RouteOverlay::endSwap()
{
    m_gc.clear();
    m_transientNodesGC.clear();
}

const std::vector<OSM::Node>* RouteOverlay::transientNodes() const
{
    return &m_transientNodes;
}

#include "moc_routeoverlay.cpp"
