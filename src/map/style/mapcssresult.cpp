/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "mapcssresult_p.h"

#include <osm/datatypes.h>

#include <algorithm>

using namespace KOSMIndoorMap;

MapCSSResultItem::MapCSSResultItem() = default;
MapCSSResultItem::~MapCSSResultItem() = default;

void MapCSSResultItem::clear()
{
    m_declarations.clear();
    m_classes.clear();
    m_tags.clear();
    m_flags = MapCSSDeclaration::NoFlag;
    m_layer = {};
}

bool MapCSSResultItem::hasAreaProperties() const
{
    return m_flags & MapCSSDeclaration::AreaProperty;
}

bool MapCSSResultItem::hasLineProperties() const
{
    return m_flags & MapCSSDeclaration::LineProperty;
}

bool MapCSSResultItem::hasLabelProperties() const
{
    return m_flags & MapCSSDeclaration::LabelProperty;
}

bool MapCSSResultItem::hasExtrudeProperties() const
{
    return m_flags & MapCSSDeclaration::ExtrudeProperty;
}

const MapCSSDeclaration* MapCSSResultItem::declaration(MapCSSDeclaration::Property prop) const
{
    const auto it = std::lower_bound(m_declarations.begin(), m_declarations.end(), prop, [](auto lhs, auto rhs) {
        return lhs->property() < rhs;
    });
    if (it == m_declarations.end() || (*it)->property() != prop) {
        return nullptr;
    }
    return (*it);
}

const std::vector<const MapCSSDeclaration*>& MapCSSResultItem::declarations() const
{
    return m_declarations;
}

void MapCSSResultItem::addDeclaration(const MapCSSDeclaration *decl)
{
    const auto it = std::lower_bound(m_declarations.begin(), m_declarations.end(), decl, [](auto lhs, auto rhs) {
        return lhs->property() < rhs->property();
    });
    if (it == m_declarations.end() || (*it)->property() != decl->property()) {
        m_declarations.insert(it, decl);
    } else {
        (*it) = decl;
    }

    m_flags |= decl->propertyFlags();
}

void MapCSSResultItem::addClass(ClassSelectorKey cls)
{
    const auto it = std::lower_bound(m_classes.begin(), m_classes.end(), cls);
    if (it == m_classes.end() || (*it) != cls) {
        m_classes.insert(it, cls);
    }
}

bool MapCSSResultItem::hasClass(ClassSelectorKey cls) const
{
    return std::binary_search(m_classes.begin(), m_classes.end(), cls);
}

LayerSelectorKey MapCSSResultItem::layerSelector() const
{
    return m_layer;
}

QByteArray MapCSSResultItem::tagValue(OSM::TagKey key) const
{
    const auto it = std::lower_bound(m_tags.begin(), m_tags.end(), key);
    if (it != m_tags.end() && (*it).key == key) {
        return (*it).value;
    }
    return {};
}

void MapCSSResultItem::setLayerSelector(LayerSelectorKey layer)
{
    m_layer = layer;
}

void MapCSSResultItem::setTag(OSM::Tag &&tag)
{
    const auto it = std::lower_bound(m_tags.begin(), m_tags.end(), tag);
    if (it == m_tags.end() || (*it).key != tag.key) {
        m_tags.insert(it, std::move(tag));
    } else {
        (*it) = std::move(tag);
    }
}


MapCSSResult::MapCSSResult() = default;
MapCSSResult::~MapCSSResult() = default;

void MapCSSResult::clear()
{
    std::move(m_results.begin(), m_results.end(), std::back_inserter(m_inactivePool));
    m_results.clear();
    std::for_each(m_inactivePool.begin(), m_inactivePool.end(), std::mem_fn(&MapCSSResultItem::clear));
}

const std::vector<MapCSSResultItem>& MapCSSResult::results() const
{
    return m_results;
}

MapCSSResultItem& MapCSSResult::operator[](LayerSelectorKey layer)
{
    const auto it = std::find_if(m_results.begin(), m_results.end(), [layer](const auto &res) {
        return res.layerSelector() == layer;
    });
    if (it != m_results.end()) {
        return *it;
    }

    if (!m_inactivePool.empty()) {
        auto res = std::move(m_inactivePool.back());
        m_inactivePool.pop_back();
        res.setLayerSelector(layer);
        m_results.push_back(std::move(res));
    } else {
        MapCSSResultItem res;
        res.setLayerSelector(layer);
        m_results.push_back(std::move(res));
    }
    return m_results.back();
}

const MapCSSResultItem& MapCSSResult::operator[](LayerSelectorKey layer) const
{
    const auto it = std::find_if(m_results.begin(), m_results.end(), [layer](const auto &res) {
        return res.layerSelector() == layer;
    });
    if (it != m_results.end()) {
        return *it;
    }

    if (m_inactivePool.empty()) {
        m_inactivePool.push_back(MapCSSResultItem());
    }
    return m_inactivePool.back();
}

void MapCSSResult::applyDeclarations(LayerSelectorKey layer, const std::vector<std::unique_ptr<MapCSSDeclaration>> &declarations)
{
    auto &result = (*this)[layer];
    for (const auto &decl : declarations) {
        switch (decl->type()) {
            case MapCSSDeclaration::PropertyDeclaration:
                result.addDeclaration(decl.get());
                break;
            case MapCSSDeclaration::ClassDeclaration:
                result.addClass(decl->classSelectorKey());
                break;
            case MapCSSDeclaration::TagDeclaration:
                if (!std::isnan(decl->doubleValue())) {
                    result.setTag(OSM::Tag{decl->tagKey(), QByteArray::number(decl->doubleValue())});
                } else {
                    result.setTag(OSM::Tag{decl->tagKey(), decl->stringValue().toUtf8()});
                }
                break;
        }
    }
}
