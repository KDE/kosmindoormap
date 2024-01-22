/*
    SPDX-FileCopyrightText: 2024 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KOSMINDOORROUTING_NAVMESHTRANSFORM_H
#define KOSMINDOORROUTING_NAVMESHTRANSFORM_H

#include "kosmindoorrouting_export.h"

#include <osm/datatypes.h>

#include <QTransform>

#include <cmath>

namespace KOSMIndoorRouting {

/** 3D vector type compatible with the Recast API. */
struct rcVec3 {
    [[nodiscard]] inline operator float*() { return &x; }
    [[nodiscard]] inline operator const float*() const { return &x; }

    float x = {};
    float y = {};
    float z = {};
};

class NavMeshTransform {
public:
    void initialize(OSM::BoundingBox bbox);

    template <typename T>
    [[nodiscard]] inline T mapGeoToNav(const T &p) const
    {
        return m_transform.map(p);
    }

    [[nodiscard]] inline QPointF mapGeoToNav(OSM::Coordinate c) const
    {
        return mapGeoToNav(QPointF(c.lonF(), c.latF()));
    }

    [[nodiscard]] inline float mapHeightToNav(int floorLevel) const
    {
        // TODO
        return (float)floorLevel;
    }

    [[nodiscard]] inline rcVec3 mapGeoHeightToNav(OSM::Coordinate c, int floorLevel) const
    {
        const auto p = mapGeoToNav(c);
        return { (float)p.x(), mapHeightToNav(floorLevel), (float)p.y() };
    }

    [[nodiscard]] inline OSM::Coordinate mapNavToGeo(rcVec3 p) const
    {
        const auto c = m_transform.inverted().map(QPointF(p.x, p.z));
        return OSM::Coordinate(c.y(), c.x());
    }

    [[nodiscard]] inline int mapNavHeightToFloorLevel(float height) const
    {
        return (int)std::round(height);
    }
private:
    QTransform m_transform;
};
}

#endif
