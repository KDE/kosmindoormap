/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "mapcssresult.h"
#include "mapcssdeclaration_p.h"

#include <osm/datatypes.h>

#include <algorithm>

using namespace KOSMIndoorMap;

namespace KOSMIndoorMap {
class MapCSSResultLayerPrivate {
public:
    std::vector<const MapCSSDeclaration*> m_declarations;
    std::vector<ClassSelectorKey> m_classes;
    std::vector<OSM::Tag> m_tags;
    LayerSelectorKey m_layer;
    int m_flags = 0;
};
}

MapCSSResultLayer::MapCSSResultLayer()
    : d(new MapCSSResultLayerPrivate)
{
}
MapCSSResultLayer::MapCSSResultLayer(MapCSSResultLayer&&) noexcept = default;
MapCSSResultLayer& MapCSSResultLayer::operator=(MapCSSResultLayer&&) noexcept = default;
MapCSSResultLayer::~MapCSSResultLayer() = default;

void MapCSSResultLayer::clear()
{
    d->m_declarations.clear();
    d->m_classes.clear();
    d->m_tags.clear();
    d->m_flags = MapCSSDeclaration::NoFlag;
    d->m_layer = {};
}

bool MapCSSResultLayer::hasAreaProperties() const
{
    return d->m_flags & MapCSSDeclaration::AreaProperty;
}

bool MapCSSResultLayer::hasLineProperties() const
{
    return d->m_flags & MapCSSDeclaration::LineProperty;
}

bool MapCSSResultLayer::hasLabelProperties() const
{
    return d->m_flags & MapCSSDeclaration::LabelProperty;
}

bool MapCSSResultLayer::hasExtrudeProperties() const
{
    return d->m_flags & MapCSSDeclaration::ExtrudeProperty;
}

const MapCSSDeclaration* MapCSSResultLayer::declaration(MapCSSProperty prop) const
{
    const auto it = std::lower_bound(d->m_declarations.begin(), d->m_declarations.end(), prop, [](auto lhs, auto rhs) {
        return lhs->property() < rhs;
    });
    if (it == d->m_declarations.end() || (*it)->property() != prop) {
        return nullptr;
    }
    return (*it);
}

const std::vector<const MapCSSDeclaration*>& MapCSSResultLayer::declarations() const
{
    return d->m_declarations;
}

void MapCSSResultLayer::addDeclaration(const MapCSSDeclaration *decl)
{
    const auto it = std::lower_bound(d->m_declarations.begin(), d->m_declarations.end(), decl, [](auto lhs, auto rhs) {
        return lhs->property() < rhs->property();
    });
    if (it == d->m_declarations.end() || (*it)->property() != decl->property()) {
        d->m_declarations.insert(it, decl);
    } else {
        (*it) = decl;
    }

    d->m_flags |= decl->propertyFlags();
}

void MapCSSResultLayer::addClass(ClassSelectorKey cls)
{
    const auto it = std::lower_bound(d->m_classes.begin(), d->m_classes.end(), cls);
    if (it == d->m_classes.end() || (*it) != cls) {
        d->m_classes.insert(it, cls);
    }
}

bool MapCSSResultLayer::hasClass(ClassSelectorKey cls) const
{
    return std::binary_search(d->m_classes.begin(), d->m_classes.end(), cls);
}

LayerSelectorKey MapCSSResultLayer::layerSelector() const
{
    return d->m_layer;
}

QByteArray MapCSSResultLayer::tagValue(OSM::TagKey key) const
{
    const auto it = std::lower_bound(d->m_tags.begin(), d->m_tags.end(), key);
    if (it != d->m_tags.end() && (*it).key == key) {
        return (*it).value;
    }
    return {};
}

void MapCSSResultLayer::setLayerSelector(LayerSelectorKey layer)
{
    d->m_layer = layer;
}

void MapCSSResultLayer::setTag(OSM::Tag &&tag)
{
    const auto it = std::lower_bound(d->m_tags.begin(), d->m_tags.end(), tag);
    if (it == d->m_tags.end() || (*it).key != tag.key) {
        d->m_tags.insert(it, std::move(tag));
    } else {
        (*it) = std::move(tag);
    }
}


namespace KOSMIndoorMap {
class MapCSSResultPrivate {
public:
    std::vector<MapCSSResultLayer> m_results;
    std::vector<MapCSSResultLayer> m_inactivePool; // for reuse of already allocated result items
};
}

MapCSSResult::MapCSSResult()
    : d(new MapCSSResultPrivate)
{
}

MapCSSResult::MapCSSResult(MapCSSResult&&) noexcept = default;
MapCSSResult::~MapCSSResult() = default;
MapCSSResult& MapCSSResult::operator=(MapCSSResult&&) noexcept = default;

void MapCSSResult::clear()
{
    std::move(d->m_results.begin(), d->m_results.end(), std::back_inserter(d->m_inactivePool));
    d->m_results.clear();
    std::for_each(d->m_inactivePool.begin(), d->m_inactivePool.end(), std::mem_fn(&MapCSSResultLayer::clear));
}

const std::vector<MapCSSResultLayer>& MapCSSResult::results() const
{
    return d->m_results;
}

MapCSSResultLayer& MapCSSResult::operator[](LayerSelectorKey layer)
{
    const auto it = std::find_if(d->m_results.begin(), d->m_results.end(), [layer](const auto &res) {
        return res.layerSelector() == layer;
    });
    if (it != d->m_results.end()) {
        return *it;
    }

    if (!d->m_inactivePool.empty()) {
        auto res = std::move(d->m_inactivePool.back());
        d->m_inactivePool.pop_back();
        res.setLayerSelector(layer);
        d->m_results.push_back(std::move(res));
    } else {
        MapCSSResultLayer res;
        res.setLayerSelector(layer);
        d->m_results.push_back(std::move(res));
    }
    return d->m_results.back();
}

const MapCSSResultLayer& MapCSSResult::operator[](LayerSelectorKey layer) const
{
    const auto it = std::find_if(d->m_results.begin(), d->m_results.end(), [layer](const auto &res) {
        return res.layerSelector() == layer;
    });
    if (it != d->m_results.end()) {
        return *it;
    }

    if (d->m_inactivePool.empty()) {
        d->m_inactivePool.emplace_back();
    }
    return d->m_inactivePool.back();
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
