/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "scenecontroller.h"
#include "logging.h"
#include "render-logging.h"

#include "iconloader_p.h"
#include "penwidthutil_p.h"
#include "poleofinaccessibilityfinder_p.h"
#include "scenegeometry_p.h"
#include "openinghourscache_p.h"
#include "texturecache_p.h"
#include "../style/mapcssdeclaration_p.h"
#include "../style/mapcssstate_p.h"
#include "../style/mapcssvalue_p.h"

#include <KOSMIndoorMap/MapData>
#include <KOSMIndoorMap/MapCSSResult>
#include <KOSMIndoorMap/MapCSSStyle>
#include <KOSMIndoorMap/OverlaySource>
#include <KOSMIndoorMap/SceneGraph>
#include <KOSMIndoorMap/View>

#include <osm/element.h>
#include <osm/datatypes.h>

#include <QDebug>
#include <QElapsedTimer>
#include <QGuiApplication>
#include <QPalette>

namespace KOSMIndoorMap {
class SceneControllerPrivate
{
public:
    MapData m_data;
    const MapCSSStyle *m_styleSheet = nullptr;
    const View *m_view = nullptr;
    std::vector<QPointer<AbstractOverlaySource>> m_overlaySources;
    mutable std::vector<OSM::Element> m_hiddenElements;
    OSM::Element m_hoverElement;

    MapCSSResult m_styleResult;
    QColor m_defaultTextColor;
    QFont m_defaultFont;
    QPolygonF m_labelPlacementPath;
    TextureCache m_textureCache;
    IconLoader m_iconLoader;
    OpeningHoursCache m_openingHours;
    PoleOfInaccessibilityFinder m_piaFinder;

    OSM::TagKey m_layerTag;
    OSM::TagKey m_typeTag;
    OSM::Languages m_langs;

    bool m_dirty = true;
    bool m_overlay = false;
};
}

using namespace KOSMIndoorMap;

SceneController::SceneController() : d(new SceneControllerPrivate)
{
    d->m_langs = OSM::Languages::fromQLocale(QLocale());
}
SceneController::~SceneController() = default;

void SceneController::setMapData(const MapData &data)
{
    d->m_data = data;
    if (!d->m_data.isEmpty()) {
        d->m_layerTag = data.dataSet().tagKey("layer");
        d->m_typeTag = data.dataSet().tagKey("type");
        d->m_openingHours.setMapData(data);
    } else {
        d->m_layerTag = {};
        d->m_typeTag = {};
        d->m_openingHours.setMapData(MapData());
    }
    d->m_dirty = true;
}

void SceneController::setStyleSheet(const MapCSSStyle *styleSheet)
{
    d->m_styleSheet = styleSheet;
    d->m_dirty = true;
}

void SceneController::setView(const View *view)
{
    d->m_view = view;
    QObject::connect(view, &View::timeChanged, view, [this]() { d->m_dirty = true; });
    d->m_dirty = true;
}

void SceneController::setOverlaySources(std::vector<QPointer<AbstractOverlaySource>> &&overlays)
{
    d->m_overlaySources = std::move(overlays);
    d->m_dirty = true;
}

void SceneController::overlaySourceUpdated()
{
    // TODO we could potentially do this more fine-grained?
    d->m_dirty = true;
}

void SceneController::updateScene(SceneGraph &sg) const
{
    QElapsedTimer sgUpdateTimer;
    sgUpdateTimer.start();

    // check if we are set up completely yet (we can't rely on a defined order with QML)
    if (!d->m_view || !d->m_styleSheet) {
        return;
    }

    // check if the scene is dirty at all
    if (sg.zoomLevel() == (int)d->m_view->zoomLevel() && sg.currentFloorLevel() == d->m_view->level() && !d->m_dirty) {
        return;
    }
    sg.setZoomLevel(d->m_view->zoomLevel());
    sg.setCurrentFloorLevel(d->m_view->level());
    d->m_openingHours.setTimeRange(d->m_view->beginTime(), d->m_view->endTime());
    d->m_dirty = false;

    sg.beginSwap();
    updateCanvas(sg);

    if (d->m_data.isEmpty()) { // if we don't have map data yet, we just need to get canvas styling here
        sg.endSwap();
        return;
    }

    // find all intermediate levels below or above the currently selected "full" level
    auto it = d->m_data.levelMap().find(MapLevel(d->m_view->level()));
    if (it == d->m_data.levelMap().end()) {
        return;
    }

    auto beginIt = it;
    if (beginIt != d->m_data.levelMap().begin()) {
        do {
            --beginIt;
        } while (!(*beginIt).first.isFullLevel() && beginIt != d->m_data.levelMap().begin());
        ++beginIt;
    }

    auto endIt = it;
    for (++endIt; endIt != d->m_data.levelMap().end(); ++endIt) {
        if ((*endIt).first.isFullLevel()) {
            break;
        }
    }

    // collect elements that the overlay want to hide
    d->m_hiddenElements.clear();
    for (const auto &overlaySource : d->m_overlaySources) {
        overlaySource->hiddenElements(d->m_hiddenElements);
    }
    std::sort(d->m_hiddenElements.begin(), d->m_hiddenElements.end());

    // for each level, update or create scene graph elements, after a some basic bounding box check
    const auto geoBbox = d->m_view->mapSceneToGeo(d->m_view->sceneBoundingBox());
    for (auto it = beginIt; it != endIt; ++it) {
        for (auto e : (*it).second) {
            if (OSM::intersects(geoBbox, e.boundingBox()) && !std::binary_search(d->m_hiddenElements.begin(), d->m_hiddenElements.end(), e)) {
                updateElement(e, (*it).first.numericLevel(), sg);
            }
        }
    }

    // update overlay elements
    d->m_overlay = true;
    for (const auto &overlaySource : d->m_overlaySources) {
        overlaySource->forEach(d->m_view->level(), [this, &geoBbox, &sg](OSM::Element e, int floorLevel) {
            if (OSM::intersects(geoBbox, e.boundingBox()) && e.type() != OSM::Type::Null) {
                updateElement(e, floorLevel, sg);
            }
        });
    }
    d->m_overlay = false;

    sg.zSort();
    sg.endSwap();

    qCDebug(RenderLog) << "updated scenegraph took" << sgUpdateTimer.elapsed() << "ms";
}

void SceneController::updateCanvas(SceneGraph &sg) const
{
    sg.setBackgroundColor(QGuiApplication::palette().color(QPalette::Base));
    d->m_defaultTextColor = QGuiApplication::palette().color(QPalette::Text);
    d->m_defaultFont = QGuiApplication::font();

    MapCSSState state;
    state.zoomLevel = d->m_view->zoomLevel();
    state.floorLevel = d->m_view->level();
    d->m_styleSheet->evaluateCanvas(state, d->m_styleResult);
    for (auto decl : d->m_styleResult[{}].declarations()) {
        switch (decl->property()) {
            case MapCSSProperty::FillColor:
                sg.setBackgroundColor(decl->colorValue());
                break;
            case MapCSSProperty::TextColor:
                d->m_defaultTextColor = decl->colorValue();
                break;
            default:
                break;
        }
    }
}

void SceneController::updateElement(OSM::Element e, int level, SceneGraph &sg) const
{
    MapCSSState state;
    state.element = e;
    state.zoomLevel = d->m_view->zoomLevel();
    state.floorLevel = d->m_view->level();
    state.openingHours = &d->m_openingHours;
    state.state = d->m_hoverElement == e ? MapCSSElementState::Hovered : MapCSSElementState::NoState;
    d->m_styleSheet->evaluate(std::move(state), d->m_styleResult);
    for (const auto &result : d->m_styleResult.results()) {
        updateElement(e, level, sg, result);
    }
}

[[nodiscard]] static bool canWordWrap(const QString &s)
{
    return std::any_of(s.begin(), s.end(), [](QChar c) { return !c.isLetter(); });
}

void SceneController::updateElement(OSM::Element e, int level, SceneGraph &sg, const MapCSSResultLayer &result) const
{
    if (result.hasAreaProperties()) {
        PolygonBaseItem *item = nullptr;
        std::unique_ptr<SceneGraphItemPayload> baseItem;
        if (e.type() == OSM::Type::Relation && e.tagValue(d->m_typeTag) == "multipolygon") {
            baseItem = sg.findOrCreatePayload<MultiPolygonItem>(e, level, result.layerSelector());
            auto i = static_cast<MultiPolygonItem*>(baseItem.get());
            if (i->path.isEmpty()) {
                i->path = createPath(e, d->m_labelPlacementPath);
            } else if (result.hasLabelProperties()) {
                SceneGeometry::outerPolygonFromPath(i->path, d->m_labelPlacementPath);
            }
            item = i;
        } else {
            baseItem = sg.findOrCreatePayload<PolygonItem>(e, level, result.layerSelector());
            auto i = static_cast<PolygonItem*>(baseItem.get());
            if (i->polygon.isEmpty()) {
                i->polygon = createPolygon(e);
            }
            d->m_labelPlacementPath = i->polygon;
            item = i;
        }

        double lineOpacity = 1.0;
        double casingOpacity = 1.0;
        double fillOpacity = 1.0;
        bool hasTexture = false;
        item->z = 0;
        initializePen(item->pen);
        initializePen(item->casingPen);
        for (auto decl : result.declarations()) {
            applyGenericStyle(decl, item);
            applyPenStyle(e, decl, item->pen, lineOpacity, item->penWidthUnit);
            applyCasingPenStyle(e, decl, item->casingPen, casingOpacity, item->casingPenWidthUnit);
            switch (decl->property()) {
                case MapCSSProperty::FillColor:
                    item->fillBrush.setColor(decl->colorValue());
                    item->fillBrush.setStyle(Qt::SolidPattern);
                    break;
                case MapCSSProperty::FillOpacity:
                    fillOpacity = decl->doubleValue();
                    break;
                case MapCSSProperty::FillImage:
                    item->textureBrush.setTextureImage(d->m_textureCache.image(decl->stringValue()));
                    hasTexture = true;
                    break;
                default:
                    break;
            }
        }
        finalizePen(item->pen, lineOpacity);
        finalizePen(item->casingPen, casingOpacity);
        if (item->fillBrush.style() == Qt::SolidPattern && item->textureBrush.style() == Qt::NoBrush && fillOpacity < 1.0) {
            auto c = item->fillBrush.color();
            c.setAlphaF(c.alphaF() * fillOpacity);
            item->fillBrush.setColor(c);
        }
        if (item->fillBrush.color().alphaF() == 0.0) {
            item->fillBrush.setStyle(Qt::NoBrush);
        }
        if (hasTexture && item->textureBrush.style() != Qt::NoBrush && fillOpacity > 0.0) {
            auto c = item->textureBrush.color();
            c.setAlphaF(fillOpacity);
            item->textureBrush.setColor(c);
        } else {
            item->textureBrush.setStyle(Qt::NoBrush);
        }

        addItem(sg, e, level, result, std::move(baseItem));
    } else if (result.hasLineProperties()) {
        auto baseItem = sg.findOrCreatePayload<PolylineItem>(e, level, result.layerSelector());
        auto item = static_cast<PolylineItem*>(baseItem.get());
        if (item->path.isEmpty()) {
            item->path = createPolygon(e);
        }

        double lineOpacity = 1.0;
        double casingOpacity = 1.0;
        item->z = 0;
        initializePen(item->pen);
        initializePen(item->casingPen);
        for (auto decl : result.declarations()) {
            applyGenericStyle(decl, item);
            applyPenStyle(e, decl, item->pen, lineOpacity, item->penWidthUnit);
            applyCasingPenStyle(e, decl, item->casingPen, casingOpacity, item->casingPenWidthUnit);
        }
        finalizePen(item->pen, lineOpacity);
        finalizePen(item->casingPen, casingOpacity);

        d->m_labelPlacementPath = item->path;
        addItem(sg, e, level, result, std::move(baseItem));
    }

    if (result.hasLabelProperties()) {
        QString text;
        auto textDecl = result.declaration(MapCSSProperty::Text);
        if (!textDecl) {
            textDecl = result.declaration(MapCSSProperty::ShieldText);
        }

        if (textDecl) {
            if (textDecl->hasExpression()) {
                MapCSSState state;
                state.element = e;
                text = QString::fromUtf8(textDecl->evaluateExpression(state, result).asString());
            } else if (!textDecl->keyValue().isEmpty()) {
                text = QString::fromUtf8(e.tagValue(d->m_langs, textDecl->keyValue().constData()));
            } else {
                text = textDecl->stringValue();
            }
        }

        const auto iconDecl = result.declaration(MapCSSProperty::IconImage);

        if (!text.isEmpty() || iconDecl) {
            auto baseItem = sg.findOrCreatePayload<LabelItem>(e, level, result.layerSelector());
            auto item = static_cast<LabelItem*>(baseItem.get());
            item->text.setText(text);
            item->textIsSet = !text.isEmpty();
            item->textOutputSizeCache = {};
            item->font = d->m_defaultFont;
            item->color = d->m_defaultTextColor;
            item->iconSize = {};
            item->textOffset = 0;
            item->z = 0;

            double textOpacity = 1.0;
            double shieldOpacity = 1.0;
            bool forceCenterPosition = false;
            bool forceLinePosition = false;
            IconData iconData;
            for (auto decl : result.declarations()) {
                applyGenericStyle(decl, item);
                applyFontStyle(decl, item->font);
                switch (decl->property()) {
                    case MapCSSProperty::TextColor:
                        item->color = decl->colorValue();
                        break;
                    case MapCSSProperty::TextOpacity:
                        textOpacity = decl->doubleValue();
                        break;
                    case MapCSSProperty::ShieldCasingColor:
                        item->casingColor = decl->colorValue();
                        break;
                    case MapCSSProperty::ShieldCasingWidth:
                        item->casingWidth = decl->doubleValue();
                        break;
                    case MapCSSProperty::ShieldColor:
                        item->shieldColor = decl->colorValue();
                        break;
                    case MapCSSProperty::ShieldOpacity:
                        shieldOpacity = decl->doubleValue();
                        break;
                    case MapCSSProperty::ShieldFrameColor:
                        item->frameColor = decl->colorValue();
                        break;
                    case MapCSSProperty::ShieldFrameWidth:
                        item->frameWidth = decl->doubleValue();
                        break;
                    case MapCSSProperty::TextPosition:
                        switch (decl->textPosition()) {
                            case MapCSSDeclaration::Position::Line:
                                forceLinePosition = true;
                                if (d->m_labelPlacementPath.size() > 1) {
                                    item->angle = SceneGeometry::polylineMidPointAngle(d->m_labelPlacementPath);
                                }
                                break;
                            case MapCSSDeclaration::Position::Center:
                                forceCenterPosition = true;
                                break;
                            case MapCSSDeclaration::Position::NoPostion:
                                break;
                        }
                        break;
                    case MapCSSProperty::TextOffset:
                        item->textOffset = decl->doubleValue();
                        break;
                    case MapCSSProperty::MaxWidth:
                        // work around for QStaticText misbehaving when we have a max width but can't actually word-wrap
                        // far from perfect but covers the most common cases
                        if (canWordWrap(text)) {
                            item->text.setTextWidth(decl->intValue());
                        }
                        break;
                    case MapCSSProperty::IconImage:
                        if (!decl->keyValue().isEmpty()) {
                            iconData.name = QString::fromUtf8(e.tagValue(decl->keyValue().constData()));
                        } else {
                            iconData.name = decl->stringValue();
                        }
                        break;
                    case MapCSSProperty::IconHeight:
                        item->iconSize.setHeight(PenWidthUtil::penWidth(e, decl, item->iconHeightUnit));
                        break;
                    case MapCSSProperty::IconWidth:
                        item->iconSize.setWidth(PenWidthUtil::penWidth(e, decl, item->iconWidthUnit));
                        break;
                    case MapCSSProperty::IconColor:
                    {
                        const auto alpha = iconData.color.alphaF();
                        iconData.color = decl->colorValue().rgb();
                        iconData.color.setAlphaF(alpha);
                        break;
                    }
                    case MapCSSProperty::IconOpacity:
                        iconData.color.setAlphaF(decl->doubleValue());
                        break;
                    case MapCSSProperty::TextHaloColor:
                        item->haloColor = decl->colorValue();
                        break;
                    case MapCSSProperty::TextHaloRadius:
                        item->haloRadius = decl->doubleValue();
                        break;
                    case MapCSSProperty::IconAllowIconOverlap:
                        item->allowIconOverlap = decl->boolValue();
                        break;
                    case MapCSSProperty::IconAllowTextOverlap:
                        item->allowTextOverlap = decl->boolValue();
                        break;
                    default:
                        break;
                }
            }

            if (item->pos.isNull()) {
                if ((result.hasAreaProperties() || forceCenterPosition) && !forceLinePosition) {
                    // for simple enough shapes we can use the faster centroid rather than the expensive PIA
                    if (d->m_labelPlacementPath.size() > 6)  {
                        item->pos = d->m_piaFinder.find(d->m_labelPlacementPath);
                    } else {
                        item->pos = SceneGeometry::polygonCentroid(d->m_labelPlacementPath);
                    }
                } else if (result.hasLineProperties() || forceLinePosition) {
                    item->pos = SceneGeometry::polylineMidPoint(d->m_labelPlacementPath);
                }
                if (item->pos.isNull()) {
                    item->pos = d->m_view->mapGeoToScene(e.center()); // node or something failed above
                }
            }

            if (item->color.isValid() && textOpacity < 1.0) {
                auto c = item->color;
                c.setAlphaF(c.alphaF() * textOpacity);
                item->color = c;
            }
            if (item->shieldColor.isValid() && shieldOpacity < 1.0) {
                auto c = item->shieldColor;
                c.setAlphaF(c.alphaF() * shieldOpacity);
                item->shieldColor = c;
            }
            if (!iconData.name.isEmpty() && iconData.color.alphaF() > 0.0) {
                if (!iconData.color.isValid()) {
                    iconData.color = d->m_defaultTextColor;
                }
                item->icon = d->m_iconLoader.loadIcon(iconData);
                item->iconOpacity = iconData.color.alphaF();
            }
            if (!item->icon.isNull()) {
                const auto iconSourceSize = item->icon.availableSizes().at(0);
                const auto aspectRatio = (double)iconSourceSize.width() / (double)iconSourceSize.height();
                if (item->iconSize.width() <= 0.0 && item->iconSize.height() <= 0.0) {
                    item->iconSize = iconSourceSize;
                } else if (item->iconSize.width() <= 0.0) {
                    item->iconSize.setWidth(item->iconSize.height() * aspectRatio);
                } else if (item->iconSize.height() <= 0.0) {
                    item->iconSize.setHeight(item->iconSize.width() / aspectRatio);
                }
            }

            if (!item->text.text().isEmpty()) {
                QTextOption opt;
                opt.setAlignment(Qt::AlignHCenter);
                opt.setWrapMode(item->text.textWidth() > 0.0 ? QTextOption::WordWrap : QTextOption::NoWrap);
                item->text.setTextOption(opt);
                // do not use QStaticText::prepare here:
                // the vast majority of text items will likely not be shown at all for being overlapped or out of view
                // and pre-computing them is too expensive. Instead this will happen as needed on first use, for only
                // a smaller amounts at a time.
                // item->text.prepare({}, item->font);

                // discard labels that are longer than the line they are aligned with
                if (result.hasLineProperties() && d->m_labelPlacementPath.size() > 1 && item->angle != 0.0) {
                    const auto sceneLen = SceneGeometry::polylineLength(d->m_labelPlacementPath);
                    const auto sceneP1 = d->m_view->viewport().topLeft();
                    const auto sceneP2 = QPointF(sceneP1.x() + sceneLen, sceneP1.y());
                    const auto screenP1 = d->m_view->mapSceneToScreen(sceneP1);
                    const auto screenP2 = d->m_view->mapSceneToScreen(sceneP2);
                    const auto screenLen = screenP2.x() - screenP1.x();
                    if (screenLen < item->text.size().width()) {
                        item->text = {};
                    }
                }

                // put texts below icons by default
                if (!item->icon.isNull() && item->textOffset == 0.0) {
                    item->textOffset = item->iconSize.height(); // ### what about heights in meters?
                }
            }

            if (!item->icon.isNull() || !item->text.text().isEmpty()) {
                addItem(sg, e, level, result, std::move(baseItem));
            }
        }
    }
}

QPolygonF SceneController::createPolygon(OSM::Element e) const
{
    const auto path = e.outerPath(d->m_data.dataSet());
    if (path.empty()) {
        return {};
    }

    QPolygonF poly;
    // Element::outerPath takes care of re-assembling broken up line segments
    // the below takes care of properly merging broken up polygons
    for (auto it = path.begin(); it != path.end();) {
        QPolygonF subPoly;
        subPoly.reserve(path.size());
        OSM::Id pathBegin = (*it)->id;

        auto subIt = it;
        for (; subIt != path.end(); ++subIt) {
            subPoly.push_back(d->m_view->mapGeoToScene((*subIt)->coordinate));
            if ((*subIt)->id == pathBegin && subIt != it && subIt != std::prev(path.end())) {
                ++subIt;
                break;
            }
        }
        it = subIt;
        poly = poly.isEmpty() ? std::move(subPoly) : poly.united(subPoly);
    }
    return poly;
}

// @see https://wiki.openstreetmap.org/wiki/Relation:multipolygon
QPainterPath SceneController::createPath(const OSM::Element e, QPolygonF &outerPath) const
{
    assert(e.type() == OSM::Type::Relation);
    outerPath = createPolygon(e); // TODO this is actually not correct for the multiple outer polygon case
    QPainterPath path;
    path.setFillRule(Qt::OddEvenFill);

    for (const auto &mem : e.relation()->members) {
        const bool isInner = std::strcmp(mem.role().name(), "inner") == 0;
        const bool isOuter = std::strcmp(mem.role().name(), "outer") == 0;
        if (mem.type() != OSM::Type::Way || (!isInner && !isOuter)) {
            continue;
        }
        if (auto way = d->m_data.dataSet().way(mem.id)) {
            const auto subPoly = createPolygon(OSM::Element(way));
            if (subPoly.isEmpty()) {
                continue;
            }
            path.addPolygon(subPoly);
            path.closeSubpath();
        }
    }

    return path;
}

void SceneController::applyGenericStyle(const MapCSSDeclaration *decl, SceneGraphItemPayload *item) const
{
    if (decl->property() == MapCSSProperty::ZIndex) {
        item->z = decl->intValue();
    }
}

void SceneController::applyPenStyle(OSM::Element e, const MapCSSDeclaration *decl, QPen &pen, double &opacity, Unit &unit) const
{
    switch (decl->property()) {
        case MapCSSProperty::Color:
            pen.setColor(decl->colorValue());
            break;
        case MapCSSProperty::Width:
            pen.setWidthF(PenWidthUtil::penWidth(e, decl, unit));
            break;
        case MapCSSProperty::Dashes:
            pen.setDashPattern(decl->dashesValue());
            break;
        case MapCSSProperty::LineCap:
            pen.setCapStyle(decl->capStyle());
            break;
        case MapCSSProperty::LineJoin:
            pen.setJoinStyle(decl->joinStyle());
            break;
        case MapCSSProperty::Opacity:
            opacity = decl->doubleValue();
            break;
        default:
            break;
    }
}

void SceneController::applyCasingPenStyle(OSM::Element e, const MapCSSDeclaration *decl, QPen &pen, double &opacity, Unit &unit) const
{
    switch (decl->property()) {
        case MapCSSProperty::CasingColor:
            pen.setColor(decl->colorValue());
            break;
        case MapCSSProperty::CasingWidth:
            pen.setWidthF(PenWidthUtil::penWidth(e, decl, unit));
            break;
        case MapCSSProperty::CasingDashes:
            pen.setDashPattern(decl->dashesValue());
            break;
        case MapCSSProperty::CasingLineCap:
            pen.setCapStyle(decl->capStyle());
            break;
        case MapCSSProperty::CasingLineJoin:
            pen.setJoinStyle(decl->joinStyle());
            break;
        case MapCSSProperty::CasingOpacity:
            opacity = decl->doubleValue();
            break;
        default:
            break;
    }
}

void SceneController::applyFontStyle(const MapCSSDeclaration *decl, QFont &font) const
{
    switch (decl->property()) {
        case MapCSSProperty::FontFamily:
            font.setFamily(decl->stringValue());
            break;
        case MapCSSProperty::FontSize:
            if (decl->unit() == MapCSSDeclaration::Pixels) {
                font.setPixelSize(decl->doubleValue());
            } else {
                font.setPointSizeF(decl->doubleValue());
            }
            break;
        case MapCSSProperty::FontWeight:
            font.setBold(decl->isBoldStyle());
            break;
        case MapCSSProperty::FontStyle:
            font.setItalic(decl->isItalicStyle());
            break;
        case MapCSSProperty::FontVariant:
            font.setCapitalization(decl->capitalizationStyle());
            break;
        case MapCSSProperty::TextDecoration:
            font.setUnderline(decl->isUnderlineStyle());
            break;
        case MapCSSProperty::TextTransform:
            font.setCapitalization(decl->capitalizationStyle());
            break;
        default:
            break;
    }
}

void SceneController::initializePen(QPen &pen) const
{
    pen.setColor(Qt::transparent);
    pen.setWidthF(0.0);

    // default according to spec
    pen.setCapStyle(Qt::FlatCap);
    pen.setJoinStyle(Qt::RoundJoin);
    pen.setStyle(Qt::SolidLine);
}

void SceneController::finalizePen(QPen &pen, double opacity) const
{
    if (pen.color().isValid() && opacity < 1.0) {
        auto c = pen.color();
        c.setAlphaF(c.alphaF() * opacity);
        pen.setColor(c);
    }

    if (pen.color().alphaF() == 0.0 || pen.widthF() == 0.0) {
        pen.setStyle(Qt::NoPen); // so the renderer can skip this entirely
    }

    // normalize dash pattern, as QPainter scales that with the line width
    if (pen.widthF() > 0.0 && !pen.dashPattern().isEmpty()) {
        auto dashes = pen.dashPattern();
        std::for_each(dashes.begin(), dashes.end(), [pen](double &d) { d /= pen.widthF(); });
        pen.setDashPattern(std::move(dashes));
    }
}

void SceneController::addItem(SceneGraph &sg, OSM::Element e, int level, const MapCSSResultLayer &result, std::unique_ptr<SceneGraphItemPayload> &&payload) const
{
    SceneGraphItem item;
    item.element = e;
    item.layerSelector = result.layerSelector();
    item.level = level;
    item.payload = std::move(payload);

    // get the OSM layer, if set
    if (!d->m_overlay) {
        auto layerStr = result.tagValue(d->m_layerTag);
        if (layerStr.isNull()) {
            layerStr = e.tagValue(d->m_layerTag);
        }
        if (!layerStr.isEmpty()) {
            bool success = false;
            const auto layer = layerStr.toInt(&success);
            if (success) {

                // ### Ignore layer information when it matches the level
                // This is very wrong according to the specification, however it looks that in many places
                // layer and level tags aren't correctly filled, possibly a side-effect of layer pre-dating
                // level and layers not having been properly updated when retrofitting level information
                // Strictly following the MapCSS rendering order yields sub-optimal results in that case, with
                // relevant elements being hidden.
                //
                // Ideally we find a way to detect the presence of that problem, and only then enabling this
                // workaround, but until we have this, this seems to produce better results in all tests.
                if (level != layer * 10) {
                    item.layer = layer;
                }
            } else {
                qCWarning(Log) << "Invalid layer:" << e.url() << layerStr;
            }
        }
    } else {
        item.layer = std::numeric_limits<int>::max();
    }

    sg.addItem(std::move(item));
}

OSM::Element SceneController::hoveredElement() const
{
    return d->m_hoverElement;
}

void SceneController::setHoveredElement(OSM::Element element)
{
    if (d->m_hoverElement == element) {
        return;
    }
    d->m_hoverElement = element;
    d->m_dirty = true;
}
