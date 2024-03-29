/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KOSMINDOORMAP_MAPCSSSTATE_P_H
#define KOSMINDOORMAP_MAPCSSSTATE_P_H

#include "mapcsselementstate.h"
#include "mapcssobjecttype_p.h"

#include <scene/openinghourscache_p.h>

#include <osm/element.h>

namespace KOSMIndoorMap {

/** Style evaluation state. */
class MapCSSState
{
public:
    OSM::Element element;
    double zoomLevel = 0.0;
    int floorLevel = 0;
    MapCSSElementStates state = {};
    MapCSSObjectType objectType = MapCSSObjectType::Any; // internal, set by MapCSSStyle
    OpeningHoursCache *openingHours = nullptr;
};

}

#endif // KOSMINDOORMAP_MAPCSSSTATE_P_H
