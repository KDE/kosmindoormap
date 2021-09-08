/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "poleofinaccessibilityfinder_p.h"
#include "scenegeometry_p.h"

#include <QDebug>
#include <QLineF>
#include <QPolygonF>

#include <cmath>
#include <limits>

using namespace KOSMIndoorMap;

/** Signed distance from @p point to @p polygon, sign indicates inside/outside distance. */
static double pointToPolygonDistance(const QPointF &point, const QPolygonF &poly)
{
    auto dist = std::numeric_limits<double>::max();
    for (auto i = 0; i < poly.size(); ++i) {
        const auto p1 = poly.at(i);
        const auto p2 = poly.at((i + 1) % poly.size());
        dist = std::min(dist, SceneGeometry::distanceToLine(QLineF(p1, p2), point));
    }
    return poly.containsPoint(point, Qt::OddEvenFill) ? dist : -dist;
}

void PoleOfInaccessibilityFinder::CellPriorityQueue::clear()
{
    c.clear();
}


PoleOfInaccessibilityFinder::Cell::Cell(const QPointF &_center, double _size, const QPolygonF &poly)
    : center(_center)
    , size(_size)
    , distance(pointToPolygonDistance(_center, poly))
{
}

double PoleOfInaccessibilityFinder::Cell::maximumDistance() const
{
    return distance + size * std::sqrt(2.0);
}

bool PoleOfInaccessibilityFinder::Cell::operator<(const PoleOfInaccessibilityFinder::Cell &other) const
{
    return maximumDistance() < other.maximumDistance();
}


PoleOfInaccessibilityFinder::PoleOfInaccessibilityFinder() = default;
PoleOfInaccessibilityFinder::~PoleOfInaccessibilityFinder() = default;

QPointF PoleOfInaccessibilityFinder::find(const QPolygonF &poly)
{
    const auto boundingBox = poly.boundingRect();
    const auto cellSize = std::min(boundingBox.width(), boundingBox.height());
    auto h = cellSize / 2.0;
    if (cellSize == 0.0) {
        return boundingBox.center();
    }

    // cover polygon with initial cells
    for (auto x = boundingBox.left(); x < boundingBox.right(); x += cellSize) {
        for (auto y = boundingBox.top(); y < boundingBox.bottom(); y += cellSize) {
            m_queue.push(Cell(QPointF(x + h, y + h), h, poly));
        }
    }

    // initial guesses
    auto bestCell = Cell(SceneGeometry::polygonCentroid(poly), 0, poly);
    const auto bboxCell = Cell(boundingBox.center(), 0, poly);
    if (bboxCell.distance > bestCell.distance) {
        bestCell = bboxCell;
    }

    while (!m_queue.empty()) {
        auto cell = m_queue.top();
        m_queue.pop();

        if (cell.distance > bestCell.distance) {
            bestCell = cell;
        }
        // don't recurse into cells that can't provide a better result
        if (cell.maximumDistance() - bestCell.distance <= 0.00002) {
            continue;
        }

        h = cell.size / 2.0;
        m_queue.push(Cell(QPointF(cell.center.x() - h, cell.center.y() - h), h, poly));
        m_queue.push(Cell(QPointF(cell.center.x() + h, cell.center.y() - h), h, poly));
        m_queue.push(Cell(QPointF(cell.center.x() - h, cell.center.y() + h), h, poly));
        m_queue.push(Cell(QPointF(cell.center.x() + h, cell.center.y() + h), h, poly));
    }

    m_queue.clear();
    return bestCell.center;
}
