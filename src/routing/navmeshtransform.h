/*
    SPDX-FileCopyrightText: 2024 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KOSMINDOORROUTING_NAVMESHTRANSFORM_H
#define KOSMINDOORROUTING_NAVMESHTRANSFORM_H

#include "kosmindoorrouting_export.h"

#include <osm/datatypes.h>

#include <QTransform>

namespace KOSMIndoorRouting {

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

    [[nodiscard]] inline double mapHeightToNav(int floorLevel) const
    {
        // TODO
        return floorLevel;
    }
private:
    QTransform m_transform;
};
}

#endif
