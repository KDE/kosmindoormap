/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef OSM_GEOMATH_H
#define OSM_GEOMATH_H

#include "kosm_export.h"

#include "datatypes.h"

#include <cmath>

namespace OSM {

/** Degree to radian conversion. */
[[nodiscard]] constexpr inline double degToRad(double deg)
{
    return deg / 180.0 * M_PI;
}
/** Radian to degree conversion. */
[[nodiscard]] constexpr inline double radToDeg(double rad)
{
    return rad / M_PI * 180.0;
}

/** Distance between two coordinates. */
[[nodiscard]] KOSM_EXPORT double distance(double lat1, double lon1, double lat2, double lon2);

/** Distance between @p coord1 and @p coord2 in meter. */
[[nodiscard]] KOSM_EXPORT double distance(Coordinate coord1, Coordinate coord2);

/** Distance in meters between a line segment defined by @p l1 and @p l2 to a point @p p. */
[[nodiscard]] double distance(Coordinate l1, Coordinate l2, Coordinate p);

/** Distance between the given polygon and coordinate, in meter. */
[[nodiscard]] KOSM_EXPORT double distance(const std::vector<const OSM::Node*> &path, Coordinate coord);

}

#endif // OSM_GEOMATH_H
