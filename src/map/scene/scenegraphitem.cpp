/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "scenegraphitem.h"
#include "view.h"

#include <QDebug>

using namespace KOSMIndoorMap;

SceneGraphItemPayload::~SceneGraphItemPayload() = default;

bool SceneGraphItemPayload::inSceneSpace() const
{
    return renderPhases() & (FillPhase | StrokePhase | CasingPhase);
}

bool SceneGraphItemPayload::inHUDSpace() const
{
    return renderPhases() & LabelPhase;
}


uint8_t PolylineItem::renderPhases() const
{
    return (pen.style() != Qt::NoPen ? StrokePhase : NoPhase) | (casingPen.style() != Qt::NoPen ? CasingPhase : NoPhase);
}

QRectF PolylineItem::boundingRect([[maybe_unused]] const View *view) const
{
    auto r = path.boundingRect(); // TODO do we need to cache this?
    double w = 0.0;
    switch (penWidthUnit) {
        case Unit::Pixel:
            w += view->mapScreenDistanceToSceneDistance(pen.widthF());
            break;
        case Unit::Meter:
            w += view->mapMetersToScene(pen.widthF());
            break;
    }
    switch (casingPenWidthUnit) {
        case Unit::Pixel:
            w += view->mapScreenDistanceToSceneDistance(casingPen.widthF());
            break;
        case Unit::Meter:
            w += view->mapMetersToScene(casingPen.widthF());
            break;
    }
    w /= 2.0;
    r.adjust(-w, -w, w, w);
    return r;
}


uint8_t PolygonBaseItem::renderPhases() const
{
    return (pen.style() == Qt::NoPen ? NoPhase : StrokePhase) | (brush.style() == Qt::NoBrush ? NoPhase : FillPhase);
}

QRectF PolygonItem::boundingRect([[maybe_unused]] const View *view) const
{
    return polygon.boundingRect(); // TODO do we need to cache this?
}

QRectF MultiPolygonItem::boundingRect([[maybe_unused]] const View *view) const
{
    return path.boundingRect(); // TODO do we need to cache this?
}


uint8_t LabelItem::renderPhases() const
{
    return LabelPhase;
}

QRectF LabelItem::boundingRect([[maybe_unused]] const View *view) const
{
    QRectF bbox;
    if (!text.text().isEmpty()) {
        bbox = QRectF(QPointF(0.0, 0.0), text.size());
    }
    if (!icon.isNull()) {
        bbox.setHeight(bbox.height() + iconSize.height());
        bbox.setWidth(std::max(bbox.width(), iconSize.width()));
    }

    bbox.moveCenter(pos);
    const auto shieldSize = std::max(frameWidth, haloRadius) + casingWidth;
    bbox.adjust(-shieldSize, -shieldSize, shieldSize, shieldSize);
    return bbox;
}
