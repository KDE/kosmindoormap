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

void MapCSSStyle::compile(const OSM::DataSet &dataSet)
{
    d->m_areaKey = dataSet.tagKey("area");
    d->m_typeKey = dataSet.tagKey("type");
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
            } else {
                state.objectType = MapCSSObjectType::LineOrArea;
            }
            break;
        }
        case OSM::Type::Relation:
            state.objectType = state.element.tagValue(d->m_typeKey) == "multipolygon" ? MapCSSObjectType::Area : MapCSSObjectType::Relation;
            break;
    }
}

void MapCSSStyle::evaluate(MapCSSState &&state, MapCSSResult &result) const
{
    result.clear();
    initializeState(state);

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
