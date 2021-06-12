/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "mapcssselector_p.h"
#include "mapcsscondition_p.h"
#include "mapcssresult_p.h"
#include "mapcssstate_p.h"

#include <QDebug>
#include <QIODevice>

#include <cmath>
#include <cstring>

using namespace KOSMIndoorMap;

MapCSSSelector::MapCSSSelector() = default;
MapCSSSelector::~MapCSSSelector() = default;

MapCSSBasicSelector::MapCSSBasicSelector() = default;
MapCSSBasicSelector::~MapCSSBasicSelector() = default;

void MapCSSBasicSelector::compile(const OSM::DataSet &dataSet)
{
    m_areaKey = dataSet.tagKey("area");
    m_typeKey = dataSet.tagKey("type");
    for (const auto &c : conditions) {
        c->compile(dataSet);
    }
}

bool MapCSSBasicSelector::matches(const MapCSSState &state, MapCSSResult &result, const std::function<void(MapCSSResult&, LayerSelectorKey)> &matchCallback) const
{
    // check zoom conditions first, as this is the cheapest one and can avoid expensive tag lookups we it doesn't match
    if (m_zoomLow > 0 && state.zoomLevel < m_zoomLow) {
        return false;
    }
    if (m_zoomHigh > 0 && state.zoomLevel >= m_zoomHigh) {
        return false;
    }

    switch (objectType) {
        case Node: if (state.element.type() != OSM::Type::Node) return false; break;
        case Way: if (state.element.type() != OSM::Type::Way) return false; break;
        case Relation: if (state.element.type() != OSM::Type::Relation) return false; break;
        case Area:
            switch (state.element.type()) {
                case OSM::Type::Null:
                case OSM::Type::Node:
                    return false;
                case OSM::Type::Way:
                    if (!state.element.way()->isClosed()) {
                        return false;
                    }
                    break;
                case OSM::Type::Relation:
                    if (state.element.tagValue(m_typeKey) != "multipolygon") {
                        return false;
                    }
                    break;
            }
            break;
        case Line:
            if (state.element.type() != OSM::Type::Way || (state.element.way()->isClosed() && state.element.tagValue(m_areaKey) == "yes")) {
                return false;
            }
            break;
        case Canvas: return false;
        case Any: break;
    }

    if (!m_class.isNull() && !result[m_layer].hasClass(m_class)) {
        return false;
    }

    if (std::all_of(conditions.begin(), conditions.end(), [&state](const auto &cond) { return cond->matches(state); })) {
        matchCallback(result, m_layer);
        return true;
    }
    return false;
}

bool MapCSSBasicSelector::matchesCanvas(const MapCSSState &state) const
{
    if (objectType != Canvas) {
        return false;
    }

    if (m_zoomLow > 0 && state.zoomLevel < m_zoomLow) {
        return false;
    }
    if (m_zoomHigh > 0 && state.zoomLevel >= m_zoomHigh) {
        return false;
    }

    return std::all_of(conditions.begin(), conditions.end(), [&state](const auto &cond) { return cond->matchesCanvas(state); });
}

struct {
    const char *name;
    MapCSSBasicSelector::ObjectType type;
} static constexpr const object_type_map[] = {
    { "node", MapCSSBasicSelector::Node },
    { "way", MapCSSBasicSelector::Way },
    { "relation", MapCSSBasicSelector::Relation },
    { "area", MapCSSBasicSelector::Area },
    { "line", MapCSSBasicSelector::Line },
    { "canvas", MapCSSBasicSelector::Canvas },
    { "*", MapCSSBasicSelector::Any },
};

void MapCSSBasicSelector::write(QIODevice *out) const
{
    for (const auto &t : object_type_map) {
        if (objectType == t.type) {
            out->write(t.name);
            break;
        }
    }

    if (!m_class.isNull()) {
        out->write(".");
        out->write(m_class.name());
    }

    if (m_zoomLow > 0 || m_zoomHigh > 0) {
        out->write("|z");
        if (m_zoomLow == m_zoomHigh) {
            out->write(QByteArray::number(m_zoomLow));
        } else {
            if (m_zoomLow > 0) {
                out->write(QByteArray::number(m_zoomLow));
            }
            out->write("-");
            if (m_zoomHigh > 0) {
                out->write(QByteArray::number(m_zoomHigh));
            }
        }
    }

    for (const auto &cond : conditions) {
        cond->write(out);
    }

    if (!m_layer.isNull()) {
        out->write("::");
        out->write(m_layer.name());
    }
}

void MapCSSBasicSelector::setObjectType(const char *str, std::size_t len)
{
    for (const auto &t : object_type_map) {
        if (std::strncmp(t.name, str, std::max(std::strlen(t.name), len)) == 0) {
            objectType = t.type;
            return;
        }
    }
}

void MapCSSBasicSelector::setZoomRange(int low, int high)
{
    m_zoomLow = low;
    m_zoomHigh = high;
}

void MapCSSBasicSelector::setConditions(MapCSSConditionHolder *conds)
{
    if (!conds) {
        return;
    }
    conditions = std::move(conds->conditions);
    delete conds;
}

void MapCSSBasicSelector::setClass(ClassSelectorKey key)
{
    m_class = key;
}

void MapCSSBasicSelector::setLayer(LayerSelectorKey key)
{
    m_layer = key;
}


void MapCSSChainedSelector::compile(const OSM::DataSet &dataSet)
{
    for (const auto &s : selectors) {
        s->compile(dataSet);
    }
}

bool MapCSSChainedSelector::matches([[maybe_unused]] const MapCSSState &state, [[maybe_unused]] MapCSSResult &result,
                                    [[maybe_unused]] const std::function<void(MapCSSResult&, LayerSelectorKey)> &matchCallback) const
{
    // TODO
    return false;
}

bool MapCSSChainedSelector::matchesCanvas(const MapCSSState &state) const
{
    Q_UNUSED(state);
    // canvas cannot be chained
    return false;
}

void MapCSSChainedSelector::write(QIODevice *out) const
{
    assert(selectors.size() > 1);
    selectors[0]->write(out);
    for (auto it = std::next(selectors.begin()); it != selectors.end(); ++it) {
        out->write(" ");
        (*it)->write(out);
    }
}

MapCSSUnionSelector::MapCSSUnionSelector() = default;
MapCSSUnionSelector::~MapCSSUnionSelector() = default;

void MapCSSUnionSelector::compile(const OSM::DataSet &dataSet)
{
    for (const auto &s : selectors) {
        s->compile(dataSet);
    }
}

bool MapCSSUnionSelector::matches(const MapCSSState &state, MapCSSResult &result, const std::function<void(MapCSSResult&, LayerSelectorKey)> &matchCallback) const
{
    return std::any_of(selectors.begin(), selectors.end(), [&state, &result, &matchCallback](const auto &selector) { return selector->matches(state, result, matchCallback); });
}

bool MapCSSUnionSelector::matchesCanvas(const MapCSSState &state) const
{
    return std::any_of(selectors.begin(), selectors.end(), [&state](const auto &selector) { return selector->matchesCanvas(state); });
}

void MapCSSUnionSelector::write(QIODevice *out) const
{
    assert(selectors.size() > 1);
    selectors[0]->write(out);
    for (auto it = std::next(selectors.begin()); it != selectors.end(); ++it) {
        out->write(",\n");
        (*it)->write(out);
    }
}
