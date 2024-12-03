/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KOSMINDOORMAP_MAPCSSSTYLE_P_H
#define KOSMINDOORMAP_MAPCSSSTYLE_P_H

#include "mapcssobjecttype_p.h"
#include "mapcssstyle.h"
#include "mapcsstypes.h"

#include <osm/element.h>

#include <memory>
#include <span>
#include <vector>

namespace KOSMIndoorMap {

class MapCSSRule;

class MapCSSStylePrivate {
public:
    explicit MapCSSStylePrivate();

    std::vector<std::unique_ptr<MapCSSRule>> m_rules;
    OSM::StringKeyRegistry<ClassSelectorKey> m_classSelectorRegistry;
    OSM::StringKeyRegistry<LayerSelectorKey> m_layerSelectorRegistry;

    OSM::TagKey m_areaKey;
    OSM::TagKey m_typeKey;

    // Rules to determine whether a closed way is a line or area.
    // see https://wiki.openstreetmap.org/wiki/Area, this is not explicitly represented in the OSM data model
    // but depends on context.
    //
    // For each rule: if tag is present and matches values (empty means the tag just has to be present) we assume type.
    struct way_type_rule_t {
        OSM::TagKey tag;
        const char *tagName;
        MapCSSObjectType type;
        std::span<const char* const> values;
    };
    std::array<way_type_rule_t, 3> m_wayTypeRules;

    inline static MapCSSStylePrivate* get(MapCSSStyle *style) { return style->d.get(); }
};

}

#endif // KOSMINDOORMAP_MAPCSSSTYLE_P_H
