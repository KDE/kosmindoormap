/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KOSMINDOORMAP_POLEOFINACCESSIBILITYFINDER_H
#define KOSMINDOORMAP_POLEOFINACCESSIBILITYFINDER_H

#include <QPointF>

#include <queue>

class QPolygonF;

namespace KOSMIndoorMap {

/** Computes the Pole Of Inaccessibility of a polygon.
 *  That is, the point furthest away from the polygon outline,
 *  which is where we usually want to place a label.
 *
 *  @see https://github.com/mapbox/polylabel
 */
class PoleOfInaccessibilityFinder
{
public:
    explicit PoleOfInaccessibilityFinder();
    ~PoleOfInaccessibilityFinder();

    QPointF find(const QPolygonF &poly);

private:
    struct Cell {
        explicit Cell(const QPointF &_center, double _size, const QPolygonF &poly);

        bool operator<(const Cell &other) const;
        double maximumDistance() const;

        QPointF center;
        double size;
        double distance;
    };

    class CellPriorityQueue : public std::priority_queue<Cell, std::vector<Cell>>
    {
    public:
        void clear();
    };
    CellPriorityQueue m_queue;
};

}

#endif // KOSMINDOORMAP_POLEOFINACCESSIBILITYFINDER_H
