/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KOSMINDOORMAP_SCENECONTROLLER_H
#define KOSMINDOORMAP_SCENECONTROLLER_H

#include "kosmindoormap_export.h"

#include "scenegraphitem.h"

#include <memory>
#include <vector>

class QPolygonF;
class QString;

namespace OSM {
class Element;
}

namespace KOSMIndoorMap {

class AbstractOverlaySource;
class MapData;
class MapCSSDeclaration;
class MapCSSResultLayer;
class MapCSSStyle;
class MapCSSState;
class SceneControllerPrivate;
class SceneGraph;
class View;

/** Creates/updates the scene graph based on a given style sheet and view. */
class KOSMINDOORMAP_EXPORT SceneController
{
public:
    explicit SceneController();
    ~SceneController();

    void setMapData(const MapData &data);
    void setStyleSheet(const MapCSSStyle *styleSheet);
    void setView(const View *view);
    void setOverlaySources(std::vector<QPointer<AbstractOverlaySource>> &&overlays);
    /** Overlay dirty state tracking. */
    void overlaySourceUpdated();

    /** Set currently hovered element. */
    [[nodiscard]] OSM::Element hoveredElement() const;
    void setHoveredElement(OSM::Element element);

    /** Creates or updates @p sg based on the currently set style and view settings.
     *  When possible, provide the scene graph of the previous run to re-use scene graph elements that didn't change.
     */
    void updateScene(SceneGraph &sg) const;

private:
    void updateCanvas(SceneGraph &sg) const;
    void updateElement(OSM::Element e, int level, SceneGraph &sg) const;
    void updateElement(const MapCSSState &state, int level, SceneGraph &sg, const MapCSSResultLayer &result) const;

    [[nodiscard]] QPolygonF createPolygon(OSM::Element e) const;
    [[nodiscard]] QPainterPath createPath(OSM::Element e, QPolygonF &outerPath) const;

    void applyGenericStyle(const MapCSSDeclaration *decl, SceneGraphItemPayload *item) const;
    void applyPenStyle(OSM::Element e, const MapCSSDeclaration *decl, QPen &pen, double &opacity, Unit &unit) const;
    void applyCasingPenStyle(OSM::Element e, const MapCSSDeclaration *decl, QPen &pen, double &opacity, Unit &unit) const;
    void applyFontStyle(const MapCSSDeclaration *decl, QFont &font) const;

    void initializePen(QPen &pen) const;
    void finalizePen(QPen &pen, double opacity) const;

    void addItem(SceneGraph &sg, const MapCSSState &state, int level, const MapCSSResultLayer &result, std::unique_ptr<SceneGraphItemPayload> &&payload) const;

    std::unique_ptr<SceneControllerPrivate> d;
};

}

#endif // KOSMINDOORMAP_SCENECONTROLLER_H
