/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "hitdetector.h"

#include "../scene/scenegraph.h"
#include "../scene/scenegraphitem.h"
#include "../scene/scenegeometry_p.h"
#include "../scene/view.h"

#include <QBrush>
#include <QFontMetrics>

using namespace KOSMIndoorMap;

const SceneGraphItem* HitDetector::itemAt(QPointF pos, const SceneGraph& sg, const View* view) const
{
    auto items = itemsAt(pos, sg, view);
    if (items.empty()) {
        return nullptr;
    }
    if (items.size() == 1) {
        return items[0];
    }

    // multiple candidates
    // (1) top element is non-transparent, use that:
    const auto top = items.back();
    qDebug() << top->element.url() << itemFillAlpha(top);
    if (itemFillAlpha(top) >= 0.5f) {
        return top;
    }

    // (2) in presence of transparency, use the smallest item at this position
    std::sort(items.begin(), items.end(), [view](auto lhs, auto rhs) {
        const auto lhsBbox = lhs->payload->boundingRect(view);
        const auto rhsBbox = rhs->payload->boundingRect(view);
        return (lhsBbox.width() * lhsBbox.height()) < (rhsBbox.width() * rhsBbox.height());
    });
    return items.front();
}

std::vector<const SceneGraphItem*> HitDetector::itemsAt(QPointF pos, const SceneGraph &sg, const View *view) const
{
    std::vector<const SceneGraphItem*> result;
    for (const auto &item : sg.items()) {
        if (item.payload->renderPhases() == SceneGraphItemPayload::NoPhase || !item.payload->boundingRect(view).contains(view->mapScreenToScene(pos))) {
            continue;
        }
        if (!itemContainsPoint(item, pos, view)) {
            continue;
        }
        result.push_back(&item);
    }

    return result;
}

bool HitDetector::itemContainsPoint(const SceneGraphItem &item, QPointF screenPos, const View *view) const
{
    if (const auto i = dynamic_cast<PolygonItem*>(item.payload.get())) {
        return itemContainsPoint(i, view->mapScreenToScene(screenPos));
    }
    if (const auto i = dynamic_cast<MultiPolygonItem*>(item.payload.get())) {
        return itemContainsPoint(i, view->mapScreenToScene(screenPos));
    }
    if (const auto i = dynamic_cast<PolylineItem*>(item.payload.get())) {
        return itemContainsPoint(i, view->mapScreenToScene(screenPos), view);
    }
    if (const auto i = dynamic_cast<LabelItem*>(item.payload.get())) {
        return itemContainsPoint(i, screenPos, view);
    }

    return true;
}

bool HitDetector::itemContainsPoint(const MultiPolygonItem *item, QPointF scenePos) const
{
    return item->path.contains(scenePos);
}

bool HitDetector::itemContainsPoint(const PolygonItem *item, QPointF scenePos) const
{
    return item->polygon.containsPoint(scenePos, Qt::OddEvenFill);
}

bool HitDetector::itemContainsPoint(const PolylineItem *item, QPointF scenePos, const View *view) const
{
    if (item->path.size() < 2) {
        return false;
    }

    const auto lineWidth = view->mapMetersToScene(item->pen.widthF())
        + view->mapScreenDistanceToSceneDistance(item->casingPen.widthF());

    double dist = std::numeric_limits<double>::max();
    // TODO do we need to wrap around here for closed lines?
    for (auto it = std::next(item->path.begin()); it != item->path.end(); ++it) {
        QLineF line(*std::prev(it), *it);
        dist = std::min(dist, SceneGeometry::distanceToLine(line, scenePos));
    }

    return dist <= lineWidth;
}

bool HitDetector::itemContainsPoint(const LabelItem *item, QPointF screenPos, const View *view) const
{
    // TODO item->angle != 0
    if (item->iconHidden) {
        return false;
    }

    if (item->textHidden) {
        const auto hitBox = item->iconHitBox(view);
        return hitBox.contains(screenPos);
    }

    const auto hitBox = item->shieldHitBox(view);
    return hitBox.contains(screenPos);
}

float HitDetector::itemFillAlpha(const SceneGraphItem *item) const
{
    if (const auto i = dynamic_cast<PolygonItem*>(item->payload.get())) {
        return std::max(i->fillBrush.color().alphaF(), i->textureBrush.color().alphaF());
    }
    if (const auto i = dynamic_cast<MultiPolygonItem*>(item->payload.get())) {
        return std::max(i->fillBrush.color().alphaF(), i->textureBrush.color().alphaF());
    }
    return 1.0f;
}
