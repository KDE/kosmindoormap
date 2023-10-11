/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "painterrenderer.h"
#include "stackblur_p.h"
#include "render-logging.h"

#include <KOSMIndoorMap/SceneGraph>
#include <KOSMIndoorMap/View>

#include <QDebug>
#include <QElapsedTimer>
#include <QFontMetricsF>
#include <QGuiApplication>
#include <QImage>
#include <QLinearGradient>
#include <QPainter>

#include <cmath>

using namespace KOSMIndoorMap;

PainterRenderer::PainterRenderer() = default;
PainterRenderer::~PainterRenderer() = default;

void PainterRenderer::setPainter(QPainter *painter)
{
    m_painter = painter;
}

void PainterRenderer::render(const SceneGraph &sg, View *view)
{
    QElapsedTimer frameTimer;
    frameTimer.start();

    m_view = view;
    beginRender();
    renderBackground(sg.backgroundColor());

    for (const auto &layerOffsets : sg.layerOffsets()) {
        const auto layerBegin = sg.itemsBegin(layerOffsets);
        const auto layerEnd = sg.itemsEnd(layerOffsets);
        //qDebug() << "rendering layer" << (*layerBegin)->layer;

        // select elements currently in view
        m_renderBatch.clear();
        m_renderBatch.reserve(layerOffsets.second - layerOffsets.first);
        const QRectF screenRect(QPointF(0, 0), QSizeF(m_view->screenWidth(), m_view->screenHeight()));
        for (auto it = layerBegin; it != layerEnd; ++it) {
            if ((*it).payload->inSceneSpace() && m_view->viewport().intersects((*it).payload->boundingRect(view))) {
                m_renderBatch.push_back((*it).payload.get());
            }
            if ((*it).payload->inHUDSpace()) {
                auto bbox = (*it).payload->boundingRect(view);
                bbox.moveCenter(m_view->mapSceneToScreen(bbox.center()));
                if (screenRect.intersects(bbox)) {
                    m_renderBatch.push_back((*it).payload.get());
                }
            }
        }

        for (auto phase : {SceneGraphItemPayload::FillPhase, SceneGraphItemPayload::CasingPhase, SceneGraphItemPayload::StrokePhase, SceneGraphItemPayload::LabelPhase}) {
            beginPhase(phase);
            for (const auto item : m_renderBatch) {
                if ((item->renderPhases() & phase) == 0) {
                    continue;
                }

                if (auto i = dynamic_cast<PolygonItem*>(item)) {
                    renderPolygon(i, phase);
                } else if (auto i = dynamic_cast<MultiPolygonItem*>(item)) {
                    renderMultiPolygon(i, phase);
                } else if (auto i = dynamic_cast<PolylineItem*>(item)) {
                    renderPolyline(i, phase);
                } else if (auto i = dynamic_cast<LabelItem*>(item)) {
                    renderLabel(i);
                } else {
                    qCritical() << "Unsupported scene graph item!";
                }
            }
        }
    }

    renderForeground(sg.backgroundColor());
    endRender();
    m_view = nullptr;

    qCDebug(RenderLog) << "rendering took:" << frameTimer.elapsed() << "ms for" << sg.items().size() << "items on" << sg.layerOffsets().size() << "layers";
}

void PainterRenderer::beginRender()
{
    m_painter->save();
}

void PainterRenderer::renderBackground(const QColor &bgColor)
{
    m_painter->setTransform(m_view->deviceTransform());
    m_painter->fillRect(0, 0, m_view->screenWidth(), m_view->screenHeight(), bgColor);
}

void PainterRenderer::beginPhase(SceneGraphItemPayload::RenderPhase phase)
{
    switch (phase) {
        case SceneGraphItemPayload::NoPhase:
            Q_UNREACHABLE();
        case SceneGraphItemPayload::FillPhase:
            m_painter->setPen(Qt::NoPen);
            m_painter->setTransform(m_view->sceneToScreenTransform() * m_view->deviceTransform());
            m_painter->setClipRect(m_view->viewport().intersected(m_view->sceneBoundingBox()));
            m_painter->setRenderHint(QPainter::Antialiasing, false);
            break;
        case SceneGraphItemPayload::CasingPhase:
        case SceneGraphItemPayload::StrokePhase:
            m_painter->setBrush(Qt::NoBrush);
            m_painter->setTransform(m_view->sceneToScreenTransform() * m_view->deviceTransform());
            m_painter->setClipRect(m_view->viewport().intersected(m_view->sceneBoundingBox()));
            m_painter->setRenderHint(QPainter::Antialiasing, true);
            break;
        case SceneGraphItemPayload::LabelPhase:
            m_painter->setTransform(m_view->deviceTransform());
            m_painter->setRenderHint(QPainter::Antialiasing, true);
            m_painter->setRenderHint(QPainter::SmoothPixmapTransform, true);
            break;
    }
}

static inline void drawGeometry(QPainter *painter, const QPolygonF &polygon) { painter->drawPolygon(polygon, Qt::OddEvenFill); }
static inline void drawGeometry(QPainter *painter, const QPainterPath &path) { painter->drawPath(path); }

template <typename T>
inline void PainterRenderer::renderPolygonFill(PolygonBaseItem *item, const T &geom)
{
    if (item->fillBrush.style() != Qt::NoBrush) {
        m_painter->setBrush(item->fillBrush);
        drawGeometry(m_painter, geom);
    }
    if (item->textureBrush.style() != Qt::NoBrush) {
        item->textureBrush.setTransform(brushTransform());
        m_painter->setOpacity(item->textureBrush.color().alphaF());
        m_painter->setBrush(item->textureBrush);
        drawGeometry(m_painter, geom);
        m_painter->setOpacity(1.0);
    }
}

template <typename T>
inline void PainterRenderer::renderPolygonCasing(PolygonBaseItem *item, const T &geom)
{
    auto p = item->casingPen;
    p.setWidthF(mapToSceneWidth(item->casingPen.widthF(), item->casingPenWidthUnit));
    m_painter->setPen(p);
    drawGeometry(m_painter, geom);
}

template <typename T>
inline void PainterRenderer::renderPolygonLine(PolygonBaseItem *item, const T &geom)
{
    auto p = item->pen;
    p.setWidthF(mapToSceneWidth(item->pen.widthF(), item->penWidthUnit));
    m_painter->setPen(p);
    drawGeometry(m_painter, geom);
}

void PainterRenderer::renderPolygon(PolygonItem *item, SceneGraphItemPayload::RenderPhase phase)
{
    if (item->useCasingFillMode()) {
        if (phase == SceneGraphItemPayload::CasingPhase) {
            renderPolygonCasing(item, item->polygon);
        } else if (phase == SceneGraphItemPayload::StrokePhase) {
            m_painter->setPen(Qt::NoPen);
            renderPolygonFill(item, item->polygon);
            m_painter->setBrush(Qt::NoBrush);
        }
    } else {
        if (phase == SceneGraphItemPayload::FillPhase) {
            renderPolygonFill(item, item->polygon);
        } else if (phase == SceneGraphItemPayload::StrokePhase) {
            renderPolygonLine(item, item->polygon);
        }
    }
}

void PainterRenderer::renderMultiPolygon(MultiPolygonItem *item, SceneGraphItemPayload::RenderPhase phase)
{
    if (item->useCasingFillMode()) {
        if (phase == SceneGraphItemPayload::CasingPhase) {
            renderPolygonCasing(item, item->path);
        } else if (phase == SceneGraphItemPayload::StrokePhase) {
            m_painter->setPen(Qt::NoPen);
            renderPolygonFill(item, item->path);
            m_painter->setBrush(Qt::NoBrush);
        }
    } else {
        if (phase == SceneGraphItemPayload::FillPhase) {
            renderPolygonFill(item, item->path);
        } else if (phase == SceneGraphItemPayload::StrokePhase) {
            renderPolygonLine(item, item->path);
        }
    }
}

void PainterRenderer::renderPolyline(PolylineItem *item, SceneGraphItemPayload::RenderPhase phase)
{
    if (phase == SceneGraphItemPayload::StrokePhase) {
        auto p = item->pen;
        p.setWidthF(mapToSceneWidth(item->pen.widthF(), item->penWidthUnit));
        m_painter->setPen(p);
        m_painter->drawPolyline(item->path);
    } else {
        auto p = item->casingPen;
        p.setWidthF(mapToSceneWidth(item->pen.widthF(), item->penWidthUnit) + mapToSceneWidth(item->casingPen.widthF(), item->casingPenWidthUnit));
        m_painter->setPen(p);
        m_painter->drawPolyline(item->path);
    }
}

void PainterRenderer::renderLabel(LabelItem *item)
{
    m_painter->save();
    m_painter->translate(m_view->mapSceneToScreen(item->pos));
    m_painter->rotate(item->angle);

    auto box = item->boundingRect(m_view);
    box.moveCenter({0.0, 0.0});

    // compute icon output size
    QSizeF iconOutputSize;
    if (!item->icon.isNull()) {
        iconOutputSize = item->iconSize;
        if (item->iconWidthUnit == Unit::Meter) {
            iconOutputSize.setWidth(m_view->mapMetersToScreen(item->iconSize.width()));
        }
        if (item->iconHeightUnit == Unit::Meter) {
            iconOutputSize.setHeight(m_view->mapMetersToScreen(item->iconSize.height()));
        }
        box.moveTop(-iconOutputSize.height() / 2.0);
    }

    // draw shield
    // @see https://wiki.openstreetmap.org/wiki/MapCSS/0.2#Shield_properties
    auto w = item->casingWidth + item->frameWidth + 2.0;
    if (item->casingWidth > 0.0 && item->casingColor.alpha() > 0) {
        m_painter->fillRect(box.adjusted(-w, -w, w, w), item->casingColor);
    }
    w -= item->casingWidth;
    if (item->frameWidth > 0.0 && item->frameColor.alpha() > 0) {
        m_painter->fillRect(box.adjusted(-w, -w, w, w), item->frameColor);
    }
    w -= item->frameWidth;
    if (item->shieldColor.alpha() > 0) {
        m_painter->fillRect(box.adjusted(-w, -w, w, w), item->shieldColor);
    }

    // draw icon
    if (!iconOutputSize.isNull()) {
        QRectF iconRect(QPointF(-iconOutputSize.width() / 2.0, -iconOutputSize.height() / 2.0), iconOutputSize);
        m_painter->setOpacity(item->iconOpacity);
        item->icon.paint(m_painter, iconRect.toRect());
        m_painter->setOpacity(1.0);
    }
    box.moveTop(box.top() + item->textOffset);

    // draw text halo
    if (item->haloRadius > 0.0 && item->haloColor.alphaF() > 0.0) {
        const auto haloBox = box.adjusted(-item->haloRadius, -item->haloRadius, item->haloRadius, item->haloRadius);
        QImage haloBuffer(haloBox.size().toSize(), QImage::Format_ARGB32);
        haloBuffer.fill(Qt::transparent);
        QPainter haloPainter(&haloBuffer);
        haloPainter.setPen(item->haloColor);
        haloPainter.setFont(item->font);
        auto haloTextRect = box;
        haloTextRect.moveTopLeft({item->haloRadius, item->haloRadius});
        haloPainter.drawStaticText(haloTextRect.topLeft(), item->text);
        StackBlur::blur(haloBuffer, item->haloRadius);
        haloPainter.setCompositionMode(QPainter::CompositionMode_SourceIn);
        haloPainter.fillRect(haloBuffer.rect(), item->haloColor);
        m_painter->drawImage(haloBox, haloBuffer);
    }

    // draw text
    if (!item->text.text().isEmpty()) {
        m_painter->setPen(item->color);
        m_painter->setFont(item->font);
        m_painter->drawStaticText(box.topLeft(), item->text);
    }

    m_painter->restore();
}

void PainterRenderer::renderForeground(const QColor &bgColor)
{
    // fade out the map at the end of the scene box, to indicate you can't scroll further
    m_painter->setTransform(m_view->deviceTransform());
    m_painter->setClipRect(m_view->mapSceneToScreen(m_view->viewport()));
    const auto borderWidth = 10;

    QColor c(bgColor);
    c.setAlphaF(0.75);
    QLinearGradient gradient;
    gradient.setColorAt(0, bgColor);
    gradient.setColorAt(0.2, c);
    gradient.setColorAt(1, Qt::transparent);

    auto r = m_view->mapSceneToScreen(m_view->sceneBoundingBox());
    r.setBottom(r.top() + borderWidth);
    gradient.setStart(r.topLeft());
    gradient.setFinalStop(r.bottomLeft());
    m_painter->fillRect(r, gradient);

    r = m_view->mapSceneToScreen(m_view->sceneBoundingBox());
    r.setTop(r.bottom() - borderWidth);
    gradient.setStart(r.bottomLeft());
    gradient.setFinalStop(r.topLeft());
    m_painter->fillRect(r, gradient);

    r = m_view->mapSceneToScreen(m_view->sceneBoundingBox());
    r.setRight(r.left() + borderWidth);
    gradient.setStart(r.topLeft());
    gradient.setFinalStop(r.topRight());
    m_painter->fillRect(r, gradient);

    r = m_view->mapSceneToScreen(m_view->sceneBoundingBox());
    r.setLeft(r.right() - borderWidth);
    gradient.setStart(r.topRight());
    gradient.setFinalStop(r.topLeft());
    m_painter->fillRect(r, gradient);
}

void PainterRenderer::endRender()
{
    m_painter->restore();
}

double PainterRenderer::mapToSceneWidth(double width, Unit unit) const
{
    switch (unit) {
        case Unit::Pixel:
            return m_view->mapScreenDistanceToSceneDistance(width);
        case Unit::Meter:
            return m_view->mapMetersToScene(width);
    }

    return width;
}

QTransform PainterRenderer::brushTransform() const
{
    // the following is the easy solution here and produces the best quality rendering
    // m_painter->transform().inverted().translate(m_painter->transform().dx(), m_painter->transform().dy());
    // the downside however is that it's extremely irritating during continuous scaling

    // the following does basically the same as the above, but with discrete zoom levels
    // at the expense of rendering quality
    constexpr const auto TextureZoomSteps = 5.0;
    auto viewport = m_view->viewportForZoom(std::round(m_view->zoomLevel() * TextureZoomSteps) / TextureZoomSteps,
                                            QPointF(m_view->screenWidth() / 2.0, m_view->screenHeight() / 2.0));
    QTransform t;
    t.scale(viewport.width() / m_view->screenWidth(), viewport.height() / m_view->screenHeight());
    t.translate(viewport.x(), viewport.y());
    return t;

    // TODO maybe the best approach would be the best of both worlds, the second approach
    // during continuous zooming, the first one when at a fixed zoom level?
}
