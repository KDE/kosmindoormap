/*
    SPDX-FileCopyrightText: 2024 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "navmeshtransform.h"

#include <osm/geomath.h>

using namespace KOSMIndoorRouting;

void NavMeshTransform::initialize(OSM::BoundingBox bbox)
{
    const auto xScale = OSM::distance(bbox.min.lonF(), bbox.center().latF(), bbox.max.lonF(), bbox.center().latF()) / bbox.widthF();
    const auto yScale = OSM::distance(bbox.center().lonF(), bbox.min.latF(), bbox.center().lonF(), bbox.max.latF()) / bbox.heightF();

    m_transform = QTransform().scale(xScale, yScale).translate(-bbox.center().lonF(), -bbox.center().latF());
}
