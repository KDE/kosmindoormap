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
    for (const auto &c : conditions) {
        c->compile(dataSet);
    }
}

bool MapCSSBasicSelector::matches(const MapCSSState &state, MapCSSResult &result, const std::vector<std::unique_ptr<MapCSSDeclaration>> &declarations) const
{
    // check object type
    switch (m_objectType) {
        case MapCSSObjectType::Node: if (state.element.type() != OSM::Type::Node) return false; break;
        case MapCSSObjectType::Way: if (state.element.type() != OSM::Type::Way) return false; break;
        case MapCSSObjectType::Relation: if (state.element.type() != OSM::Type::Relation) return false; break;
        case MapCSSObjectType::Area: if (state.objectType !=MapCSSObjectType::Area && state.objectType != MapCSSObjectType::LineOrArea) return false; break;
        case MapCSSObjectType::Line: if (state.objectType !=MapCSSObjectType::Line && state.objectType != MapCSSObjectType::LineOrArea) return false; break;
        case MapCSSObjectType::Canvas: return false;
        case MapCSSObjectType::Any: break;
        case MapCSSObjectType::LineOrArea: Q_UNREACHABLE();
    }

    // check zoom level
    if (m_zoomLow > 0 && state.zoomLevel < m_zoomLow) {
        return false;
    }
    if (m_zoomHigh > 0 && state.zoomLevel >= m_zoomHigh) {
        return false;
    }

    if (!m_class.isNull() && !result[m_layer].hasClass(m_class)) {
        return false;
    }

    if (std::all_of(conditions.begin(), conditions.end(), [&state](const auto &cond) { return cond->matches(state); })) {
        result.applyDeclarations(m_layer, declarations);
        return true;
    }
    return false;
}

bool MapCSSBasicSelector::matchesCanvas(const MapCSSState &state) const
{
    if (m_objectType != MapCSSObjectType::Canvas) {
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

LayerSelectorKey MapCSSBasicSelector::layerSelector() const
{
    return m_layer;
}

struct {
    const char *name;
    MapCSSObjectType type;
} static constexpr const object_type_map[] = {
    { "node", MapCSSObjectType::Node },
    { "way", MapCSSObjectType::Way },
    { "relation", MapCSSObjectType::Relation },
    { "area", MapCSSObjectType::Area },
    { "line", MapCSSObjectType::Line },
    { "canvas", MapCSSObjectType::Canvas },
    { "*", MapCSSObjectType::Any },
};

void MapCSSBasicSelector::write(QIODevice *out) const
{
    for (const auto &t : object_type_map) {
        if (m_objectType == t.type) {
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
            m_objectType = t.type;
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
                                    [[maybe_unused]] const std::vector<std::unique_ptr<MapCSSDeclaration>> &declarations) const
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

LayerSelectorKey MapCSSChainedSelector::layerSelector() const
{
    return selectors.back()->layerSelector();
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
    for (const auto &ls : m_selectors) {
        for (const auto &s : ls.selectors) {
            s->compile(dataSet);
        }
    }
}

bool MapCSSUnionSelector::matches(const MapCSSState &state, MapCSSResult &result, const std::vector<std::unique_ptr<MapCSSDeclaration>> &declarations) const
{
    bool ret = false;
    for (const auto &ls : m_selectors) {
        if (std::any_of(ls.selectors.begin(), ls.selectors.end(), [&state, &result, &declarations](const auto &selector) {
            return selector->matches(state, result, declarations);

        })) {
            // no short-circuit evaluation, we want the matchCallback to be called once per matching layer!
            ret = true;
        }
    }
    return ret;
}

bool MapCSSUnionSelector::matchesCanvas(const MapCSSState &state) const
{
    for (const auto &ls : m_selectors) {
        if (ls.layer.isNull()) {
            return std::any_of(ls.selectors.begin(), ls.selectors.end(), [&state](const auto &selector) { return selector->matchesCanvas(state); });
        }
    }
    return false;
}

LayerSelectorKey MapCSSUnionSelector::layerSelector() const
{
    return {};
}

void MapCSSUnionSelector::write(QIODevice *out) const
{
    for (std::size_t i = 0; i < m_selectors.size(); ++i) {
        for (std::size_t j = 0; j < m_selectors[i].selectors.size(); ++j) {
            if (i != 0 || j != 0) {
                out->write(",\n");
            }
            m_selectors[i].selectors[j]->write(out);
        }
    }
}

void MapCSSUnionSelector::addSelector(std::unique_ptr<MapCSSSelector> &&selector)
{
    auto it = std::find_if(m_selectors.begin(), m_selectors.end(), [&selector](const auto &ls) {
        return ls.layer == selector->layerSelector();
    });
    if (it != m_selectors.end()) {
        (*it).selectors.push_back(std::move(selector));
    } else {
        SelectorMap ls;
        ls.layer = selector->layerSelector();
        ls.selectors.push_back(std::move(selector));
        m_selectors.push_back(std::move(ls));
    }
}
