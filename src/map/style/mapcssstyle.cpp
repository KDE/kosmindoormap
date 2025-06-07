/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "mapcssstyle.h"
#include "mapcssstyle_p.h"
#include "mapcssparser.h"
#include "mapcssresult.h"
#include "mapcssrule_p.h"
#include "mapcssstate_p.h"
#include "mapcsstypes.h"

#include <QDebug>
#include <QIODevice>

using namespace KOSMIndoorMap;

// NOTE:
// - only add rules we really need, this has runtime cost
// - keep value lists alphabetically sorted
// - follow rules linked on https://wiki.openstreetmap.org/wiki/Area
static constexpr inline const char* way_rule_highway_values[] = {
    "elevator",
    "platform",
    "rest_area",
    "services"
};
static constexpr inline const char* way_rule_natural_values[] = {
    "bare_rock",
    "bay",
    "beach",
    "crevasse",
    "desert",
    "fell",
    "glacier",
    "grassland",
    "heath",
    "landslide",
    "mud",
    "reef",
    "rock",
    "sand",
    "scree",
    "scrub",
    "shingle",
    "sinkhole",
    "stone",
    "water",
    "wetland",
    "wood"
};

MapCSSStylePrivate::MapCSSStylePrivate()
    : m_wayTypeRules({
        { OSM::TagKey{}, "building", MapCSSObjectType::Area, {}},
        { OSM::TagKey{}, "building:levels", MapCSSObjectType::Area, {}},
        { OSM::TagKey{}, "building:part", MapCSSObjectType::Area, {}},
        { OSM::TagKey{}, "highway", MapCSSObjectType::Area, std::span<const char* const>{way_rule_highway_values}},
        { OSM::TagKey{}, "natural", MapCSSObjectType::Area, std::span<const char* const>{way_rule_natural_values}},
    })
{
}

MapCSSStyle::MapCSSStyle()
    : d(new MapCSSStylePrivate)
{}

MapCSSStyle::MapCSSStyle(MapCSSStyle&&) noexcept = default;
MapCSSStyle::~MapCSSStyle() = default;
MapCSSStyle& MapCSSStyle::operator=(MapCSSStyle&&) noexcept = default;

bool MapCSSStyle::isEmpty() const
{
    return d->m_rules.empty();
}

void MapCSSStyle::compile(OSM::DataSet &dataSet)
{
    d->m_areaKey = dataSet.tagKey("area");
    d->m_typeKey = dataSet.tagKey("type");

    for (auto &rule : d->m_wayTypeRules) {
        rule.tag = dataSet.tagKey(rule.tagName);
    }
    std::sort(d->m_wayTypeRules.begin(), d->m_wayTypeRules.end(), [](const auto &lhs, const auto &rhs) { return lhs.tag < rhs.tag; });

    for (const auto &rule : d->m_rules) {
        rule->compile(dataSet);
    }
}

void MapCSSStyle::initializeState(MapCSSState &state) const
{
    // determine object type of the input element
    // This involves tag lookups (and thus cost), but as long as there is at least
    // one area and one line selector for each zoom level this is break-even. In practice
    // there are actually many more than that, which means this is a useful optimization
    // over doing this in MapCSSBasicSelector after checking for the zoom level
    switch (state.element.type()) {
        case OSM::Type::Null:
            Q_UNREACHABLE();
        case OSM::Type::Node:
            state.objectType = MapCSSObjectType::Node;
            break;
        case OSM::Type::Way:
        {
            if (!state.element.way()->isClosed()) {
                state.objectType = MapCSSObjectType::Line;
                break;
            }
            const auto area = state.element.tagValue(d->m_areaKey);
            if (area == "yes") {
                state.objectType = MapCSSObjectType::Area;
            } else if (!area.isEmpty()) {
                state.objectType = MapCSSObjectType::Line;
            } else {
                state.objectType = MapCSSObjectType::LineOrArea;
                for (const auto &tag : state.element.way()->tags) {
                    auto it = std::lower_bound(d->m_wayTypeRules.begin(), d->m_wayTypeRules.end(), tag.key, [](const auto &rule, const auto &key) {
                        return rule.tag < key;
                    });
                    if (it != d->m_wayTypeRules.end() && (*it).tag == tag.key) {
                        if ((*it).values.empty()) {
                            state.objectType = (*it).type;
                        } else {
                            if (std::binary_search((*it).values.begin(), (*it).values.end(), tag.value)) {
                                state.objectType = (*it).type;
                            } else {
                                state.objectType = (*it).type == MapCSSObjectType::Line ? MapCSSObjectType::Area :MapCSSObjectType::Line;
                            }
                        }
                        break;
                    }
                }
            }
            break;
        }
        case OSM::Type::Relation:
            state.objectType = state.element.tagValue(d->m_typeKey) == "multipolygon" ? MapCSSObjectType::Area : MapCSSObjectType::Relation;
            break;
    }
}

void MapCSSStyle::evaluate(const MapCSSState &state, MapCSSResult &result) const
{
    result.clear();

    for (const auto &rule : d->m_rules) {
        rule->evaluate(state, result);
    }
}

void MapCSSStyle::evaluateCanvas(const MapCSSState &state, MapCSSResult &result) const
{
    result.clear();
    for (const auto &rule : d->m_rules) {
        rule->evaluateCanvas(state, result);
    }
}

void MapCSSStyle::write(QIODevice *out) const
{
    for (const auto &rule : d->m_rules) {
        rule->write(out);
    }
}

ClassSelectorKey MapCSSStyle::classKey(const char *className) const
{
    return d->m_classSelectorRegistry.key(className);
}

LayerSelectorKey MapCSSStyle::layerKey(const char *layerName) const
{
    return d->m_layerSelectorRegistry.key(layerName);
}
