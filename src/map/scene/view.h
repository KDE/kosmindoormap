/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KOSMINDOORMAP_VIEW_H
#define KOSMINDOORMAP_VIEW_H

#include "kosmindoormap_export.h"

#include <KOSM/Datatypes>

#include <QDateTime>
#include <QObject>
#include <QRectF>
#include <QSize>
#include <QTransform>

namespace KOSMIndoorMap {

/** View transformations and transformation manipulation.
 *  There are three different coordinate systems involved here:
 *  - The geographic world coordinates of the OSM input data.
 *    This uses OSM::Coordinate.
 *  - The scene coordinates which have a the Web Mercator projection applied (see https://en.wikipedia.org/wiki/Mercator_projection).
 *    This uses QPointF ranging from 0x0 to 256x256
 *  - The screen coordinates (ie. visible pixels on screen).
 *    This uses QPoint.
 *  Further, there's also three slight variations of those in use here:
 *  - "HUD" coordinates: elements that follow the scene coordinates for their positioning,
 *    but the screen coordinates regarding scaling and rotation. This is used for map labels.
 *  - Geographic distances. This is needed to display things in a fixed width in meters in the scene,
 *    or to compute the map scale. Note that this only works due to the relatively high zoom levels,
 *    so that earth curvature or map projection effects are negligible.
 *  - "pan space": same transform as screen space, but with the origin at the origin of the scene bounding box
 *    This is useful for implementing scene-wide panning and showing scroll bars.
 */
class KOSMINDOORMAP_EXPORT View : public QObject
{
    Q_OBJECT
    Q_PROPERTY(double panX READ panX NOTIFY transformationChanged)
    Q_PROPERTY(double panY READ panY NOTIFY transformationChanged)
    Q_PROPERTY(double panWidth READ panWidth NOTIFY transformationChanged)
    Q_PROPERTY(double panHeight READ panHeight NOTIFY transformationChanged)
    Q_PROPERTY(int floorLevel READ level WRITE setLevel NOTIFY floorLevelChanged)
    Q_PROPERTY(QRectF viewport READ viewport NOTIFY transformationChanged)
    Q_PROPERTY(double zoomLevel READ zoomLevel NOTIFY transformationChanged)
    Q_PROPERTY(QDateTime beginTime READ beginTime WRITE setBeginTime NOTIFY timeChanged)
    Q_PROPERTY(QDateTime endTime READ endTime WRITE setEndTime NOTIFY timeChanged)
public:
    explicit View(QObject *parent = nullptr);
    ~View();

    /** Map a geographic coordinate to a scene coordinate, ie. apply the mercator projection. */
    [[nodiscard]] static QPointF mapGeoToScene(OSM::Coordinate coord);
    [[nodiscard]] static QRectF mapGeoToScene(OSM::BoundingBox box);
    /** Map a scene coordinate to a geographic one, ie. apply the inverse mercator projection. */
    [[nodiscard]] static OSM::Coordinate mapSceneToGeo(QPointF p);
    Q_INVOKABLE [[nodiscard]] static OSM::BoundingBox mapSceneToGeo(const QRectF &box);

    /** Screen-space sizes, ie the size of the on-screen area used for displaying. */
    int screenWidth() const;
    int screenHeight() const;
    void setScreenSize(QSize size);

    /** The transformation to apply to scene coordinate to get to the view on screen. */
    QTransform sceneToScreenTransform() const;

    /** The (floor) level to display.
     *  @see MapLevel.
     */
    int level() const;
    void setLevel(int level);

    /** OSM-compatible zoom level, ie. the 2^level-th subdivision of the scene space. */
    double zoomLevel() const;
    /** Set the zoom level to @p zoom, and adjusting it around center position @p center. */
    Q_INVOKABLE void setZoomLevel(double zoom, QPointF screenCenter);

    /** The sub-rect of the scene bounding box currently displayed.
     *  Specified in scene coordinates.
     */
    [[nodiscard]] QRectF viewport() const;
    void setViewport(const QRectF &viewport);

    /** Computes the viewport for the given @p zoom level and @p screenCenter.
     *  This does not apply the zoom change to the view itself though.
     */
    [[nodiscard]] QRectF viewportForZoom(double zoom, QPointF screenCenter) const;

    /** The bounding box of the scene.
     *  The viewport cannot exceed this area.
     */
    QRectF sceneBoundingBox() const;
    void setSceneBoundingBox(OSM::BoundingBox bbox);
    void setSceneBoundingBox(const QRectF &bbox);

    /** Converts a point in scene coordinates to screen coordinates. */
    QPointF mapSceneToScreen(QPointF scenePos) const;
    /** Converts a rectangle in scene coordinates to screen coordinates. */
    QRectF mapSceneToScreen(const QRectF &sceneRect) const;
    /** Converts a point in screen coordinates to scene coordinates. */
    Q_INVOKABLE [[nodiscard]] QPointF mapScreenToScene(QPointF screenPos) const;
    /** Converts a distance in screen coordinates to a distance in scene coordinates. */
    double mapScreenDistanceToSceneDistance(double distance) const;

    /** Returns how many units in scene coordinate represent the distance of @p meters in the current view transformation. */
    double mapMetersToScene(double meters) const;
    /** Returns how many pixels on screen represent the distance of @p meters with the current view transformation. */
    Q_INVOKABLE double mapMetersToScreen(double meters) const;
    /** Returns how many meters are represented by @p pixels with the current view transformation. */
    Q_INVOKABLE double mapScreenToMeters(int pixels) const;

    void panScreenSpace(QPoint offset);
    /** Increase zoom level by one/scale up by 2x around the screen position @p center. */
    Q_INVOKABLE void zoomIn(QPointF screenCenter);
    /** Decrease zoom level by one/scale down by 2x around the screen position @p center. */
    Q_INVOKABLE void zoomOut(QPointF screenCenter);

    /** Position of the viewport in pan coordinates. */
    double panX() const;
    double panY() const;
    /** Size of the pan-able area in screen coordinates. */
    double panWidth() const;
    double panHeight() const;

    /** Move the viewport to the pan coordinates @p x and @p y. */
    Q_INVOKABLE void panTopLeft(double x, double y);

    /** Device tranformation for manual high DPI scaling. */
    QTransform deviceTransform() const;
    void setDeviceTransform(const QTransform &t);

    /** Center the view on the given geo-coordinate, but leave floor level and zoom unchanged. */
    Q_INVOKABLE void centerOnGeoCoordinate(QPointF geoCoord);

    /** Center on @p geoCoord on @p floorLevel with @p zoomLevel.
     *  This is a convenience method equivalent of calling setLevel, setZoomLevel on the view center
     *  and centerOnGeoCoordinate, in that order.
     *
     *  @since 25.08
     */
    Q_INVOKABLE void centerOn(QPointF geoCoord, int floorLevel, double zoomLevel);

    /** Time range that is displayed.
     *  This matters for example when opening hours are considered for styling.
     */
    QDateTime beginTime() const;
    void setBeginTime(const QDateTime &beginTime);
    QDateTime endTime() const;
    void setEndTime(const QDateTime &endTime);

Q_SIGNALS:
    void transformationChanged();
    void floorLevelChanged();
    void timeChanged();

protected:
    /** QML only API due to lack of OSM::Coordinate support there. */
    Q_INVOKABLE [[nodiscard]] static QPointF mapSceneToGeoPoint(QPointF p);

private:
    /** Ensure we stay within the bounding box with the viewport, call after viewport modification. */
    void constrainViewToScene();
    [[nodiscard]] QRectF constrainedViewport(QRectF viewport) const;

    /** Needs to be called for any update to the viewport to updates internal caches
     *  and emits change signals.
     */
    void updateViewport();

    QRectF m_bbox;
    QRectF m_viewport;
    QSize m_screenSize;
    QTransform m_deviceTransform;
    int m_level = 0;

    // cached values
    double m_screenWidthInMeters = 1.0;
    QTransform m_sceneToScreenTransform;
    QTransform m_screenToSceneTransform;

    QDateTime m_beginTime;
    QDateTime m_endTime;
};

}

#endif // KOSMINDOORMAP_VIEW_H
