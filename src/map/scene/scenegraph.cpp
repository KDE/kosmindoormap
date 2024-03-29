/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "scenegraph.h"

#include <QDebug>
#include <QGuiApplication>
#include <QPalette>

using namespace KOSMIndoorMap;

SceneGraph::SceneGraph() = default;
SceneGraph::SceneGraph(SceneGraph&&) = default;
SceneGraph::~SceneGraph() = default;
SceneGraph& SceneGraph::operator=(SceneGraph &&other) = default;

void SceneGraph::clear()
{
    m_items.clear();
    m_previousItems.clear();
    m_layerOffsets.clear();
    m_bgColor = {};
    m_floorLevel = 0;
    m_zoomLevel = 0;
}

void SceneGraph::addItem(SceneGraphItem &&item)
{
    m_items.push_back(std::move(item));
}

void SceneGraph::zSort()
{
    std::stable_sort(m_items.begin(), m_items.end(), SceneGraph::zOrderCompare);
    recomputeLayerIndex();
}

static inline uint64_t area(const SceneGraphItem &item)
{
    return (uint64_t)item.element.boundingBox().width() * (uint64_t)item.element.boundingBox().height();
}

bool SceneGraph::zOrderCompare(const SceneGraphItem &lhs, const SceneGraphItem &rhs)
{
    /* The MapCSS spec says we have to render in the following order:
     * - Objects with lower layer should always be rendered first.
     * - Within a layer, first all fills are rendered, then all casings, then all strokes, then all icons and labels.
     * - Within each of those categories, objects are ordered according to z-index.
     * - If all of the above are equal, the order is undefined.
     */
    if (lhs.level == rhs.level) {
        if (lhs.layer == rhs.layer) {
            if (lhs.payload->z == rhs.payload->z) {
                return area(lhs) > area(rhs);
            }
            return lhs.payload->z < rhs.payload->z;
        }
        return lhs.layer < rhs.layer;
    }
    return lhs.level < rhs.level;
}

void SceneGraph::beginSwap()
{
    std::swap(m_items, m_previousItems);
    m_items.clear();
    std::sort(m_previousItems.begin(), m_previousItems.end(), SceneGraph::itemPoolCompare);
    m_layerOffsets.clear();
}

void SceneGraph::endSwap()
{
    m_previousItems.clear();
}

int SceneGraph::zoomLevel() const
{
    return m_zoomLevel;
}

void SceneGraph::setZoomLevel(int zoom)
{
    m_zoomLevel = zoom;
}

int SceneGraph::currentFloorLevel() const
{
    return m_floorLevel;
}

void SceneGraph::setCurrentFloorLevel(int level)
{
    m_floorLevel = level;
}

QColor SceneGraph::backgroundColor() const
{
    return m_bgColor;
}

void SceneGraph::setBackgroundColor(const QColor &bg)
{
    m_bgColor = bg;
}

void SceneGraph::recomputeLayerIndex()
{
    m_layerOffsets.clear();
    if (m_items.empty()) {
        return;
    }

    auto prevIndex = 0;
    for (auto it = m_items.begin(); it != m_items.end();) {
        it = std::upper_bound(it, m_items.end(), *it, [](const auto &lhs, const auto &rhs) {
            if (lhs.level == rhs.level) {
                return lhs.layer < rhs.layer;
            }
            return lhs.level < rhs.level;
        });
        const auto nextIndex = std::distance(m_items.begin(), it);
        m_layerOffsets.push_back(std::make_pair(prevIndex, nextIndex));
        prevIndex = nextIndex;
    }
}

const std::vector<SceneGraph::LayerOffset>& SceneGraph::layerOffsets() const
{
    return m_layerOffsets;
}

SceneGraph::SceneGraphItemIter SceneGraph::itemsBegin(SceneGraph::LayerOffset layer) const
{
    return m_items.begin() + layer.first;
}

SceneGraph::SceneGraphItemIter SceneGraph::itemsEnd(SceneGraph::LayerOffset layer) const
{
    return m_items.begin() + layer.second;
}

const std::vector<SceneGraphItem>& SceneGraph::items() const
{
    return m_items;
}

bool SceneGraph::itemPoolCompare(const SceneGraphItem &lhs, const SceneGraphItem &rhs)
{
    if (lhs.element.type() == rhs.element.type()) {
        if (lhs.element.id() == rhs.element.id()) {
            if (lhs.layerSelector == rhs.layerSelector) {
                return lhs.level < rhs.level;
            }
            return lhs.layerSelector < rhs.layerSelector;
        }
        return lhs.element.id() < rhs.element.id();
    }
    return lhs.element.type() < rhs.element.type();
}
