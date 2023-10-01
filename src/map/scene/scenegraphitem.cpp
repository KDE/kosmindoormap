/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "scenegraphitem.h"
#include "view.h"

#include <QDebug>
#include <QFontMetrics>

using namespace KOSMIndoorMap;

SceneGraphItemPayload::~SceneGraphItemPayload() = default;

bool SceneGraphItemPayload::inSceneSpace() const
{
    return renderPhases() & (FillPhase | StrokePhase | CasingPhase);
}

bool SceneGraphItemPayload::inHUDSpace() const
{
    return renderPhases() & (IconPhase | LabelPhase);
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
    if (useCasingFillMode()) {
        return StrokePhase | CasingPhase;
    }
    return (pen.style() == Qt::NoPen ? NoPhase : StrokePhase)
        | (fillBrush.style() == Qt::NoBrush && textureBrush.style() == Qt::NoBrush ? NoPhase : FillPhase);
}

bool PolygonBaseItem::useCasingFillMode() const
{
    return casingPen.style() != Qt::NoPen && (fillBrush.style() != Qt::NoBrush || textureBrush.style() != Qt::NoBrush);
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
    if (hasShield()) {
        return IconPhase;
    }
    return (hasIcon() ? IconPhase : NoPhase) | (hasText() ? LabelPhase : NoPhase);
}

QRectF LabelItem::boundingRect(const View *view) const
{
    QRectF bbox;
    if (hasText()) {
        bbox = QRectF(QPointF(0.0, 0.0), textOutputSize());
    }
    if (hasIcon()) {
        const auto s = iconOutputSize(view);
        bbox.setHeight(bbox.height() + s.height());
        bbox.setWidth(std::max(bbox.width(), s.width()));
    }

    const auto shieldSize = casingAndFrameWidth();
    bbox.adjust(-shieldSize, -shieldSize, shieldSize, shieldSize);

    bbox.moveCenter(pos);
    return bbox;
}

QSizeF LabelItem::iconOutputSize(const View *view) const
{
    if (!hasIcon()) {
        return {};
    }

    const auto h = iconHeightUnit == Unit::Meter ? view->mapMetersToScreen(iconSize.height()) : iconSize.height();
    const auto w = iconWidthUnit == Unit::Meter ? view->mapMetersToScreen(iconSize.width()) : iconSize.width();
    return {w, h};
}

QSizeF LabelItem::textOutputSize() const
{
    if (!hasText()) {
        return {};
    }

    // QStaticText::size doesn't return the actual bounding box with QStaticText::textWidth is set,
    // so we need to compute this manually here to not end up with overly large hitboxes
    // TODO cache this
    if (text.textWidth() > 0) {
        return QFontMetrics(font).boundingRect(0, 0, (int)text.textWidth(), 1000, Qt::AlignHCenter | Qt::AlignTop | Qt::TextWordWrap, text.text()).size();
    }
    return QFontMetrics(font).boundingRect(text.text()).size();
}

double LabelItem::casingAndFrameWidth() const
{
    return std::max(frameWidth, haloRadius) + casingWidth;
}

bool LabelItem::hasIcon() const
{
    return !icon.isNull();
}

bool LabelItem::hasText() const
{
    return !text.text().isEmpty();
}

bool LabelItem::hasShield() const
{
    return (casingWidth > 0.0 && casingColor.alpha() > 0)
        || (frameWidth > 0.0 && frameColor.alpha() > 0)
        || shieldColor.alpha() > 0;
}
