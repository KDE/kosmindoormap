/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KOSMINDOORMAP_MAPCSSSTYLE_P_H
#define KOSMINDOORMAP_MAPCSSSTYLE_P_H

#include "mapcssstyle.h"
#include "mapcsstypes.h"

#include <osm/element.h>

#include <memory>
#include <vector>

namespace KOSMIndoorMap {

class MapCSSRule;

class MapCSSStylePrivate {
public:
    std::vector<std::unique_ptr<MapCSSRule>> m_rules;
    OSM::StringKeyRegistry<ClassSelectorKey> m_classSelectorRegistry;
    OSM::StringKeyRegistry<LayerSelectorKey> m_layerSelectorRegistry;

    OSM::TagKey m_areaKey;
    OSM::TagKey m_typeKey;

    inline static MapCSSStylePrivate* get(MapCSSStyle *style) { return style->d.get(); }
};

}

#endif // KOSMINDOORMAP_MAPCSSSTYLE_P_H
