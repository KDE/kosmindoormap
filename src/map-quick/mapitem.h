/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KOSMINDOORMAP_MAPITEM_H
#define KOSMINDOORMAP_MAPITEM_H

#include "osmelement.h"

#include <KOSMIndoorMap/FloorLevelModel>
#include <KOSMIndoorMap/MapData>
#include <KOSMIndoorMap/MapCSSStyle>
#include <KOSMIndoorMap/MapLoader>
#include <KOSMIndoorMap/PainterRenderer>
#include <KOSMIndoorMap/SceneController>
#include <KOSMIndoorMap/SceneGraph>
#include <KOSMIndoorMap/View>

#include <QQuickPaintedItem>

namespace KOSMIndoorMap {

class MapCSSLoader;
class MapData;

/** Map renderer for the IndoorMap QML item.
 *  @internal Do not use directly!
 */
class MapItem : public QQuickPaintedItem
{
    Q_OBJECT
    Q_PROPERTY(KOSMIndoorMap::MapLoader* loader READ loader CONSTANT)
    Q_PROPERTY(KOSMIndoorMap::View* view READ view CONSTANT)
    Q_PROPERTY(QString styleSheet READ styleSheetName WRITE setStylesheetName NOTIFY styleSheetChanged)
    Q_PROPERTY(KOSMIndoorMap::FloorLevelModel* floorLevels READ floorLevelModel CONSTANT)

    /** There's a loading error (data not found, network issue, broken style sheet, etc). */
    Q_PROPERTY(bool hasError READ hasError NOTIFY errorChanged)
    /** Details on the error. */
    Q_PROPERTY(QString errorMessage READ errorMessage NOTIFY errorChanged)

    Q_PROPERTY(KOSMIndoorMap::MapData mapData READ mapData NOTIFY mapDataChanged)

    /** Sources for overlays that should be rendered on top of the map. */
    Q_PROPERTY(QVariant overlaySources READ overlaySources WRITE setOverlaySources NOTIFY overlaySourcesChanged)

    /** ISO 3166-1/2 country or region code this map area is in.
     *  Used for interpreting opening hours expressions.
     */
    Q_PROPERTY(QString region READ region WRITE setRegion NOTIFY regionChanged)

    /** IANA timezone id of the timezone this map area is in.
     *  Used for interpreting opening hours expressions.
     */
    Q_PROPERTY(QString timeZone READ timeZoneId WRITE setTimeZoneId NOTIFY timeZoneChanged)

    /** Currently hovered element. */
    Q_PROPERTY(KOSMIndoorMap::OSMElement hoveredElement READ hoveredElement WRITE setHoveredElement NOTIFY hoveredElementChanged)

public:
    explicit MapItem(QQuickItem *parent = nullptr);
    ~MapItem();

    void paint(QPainter *painter) override;

    [[nodiscard]] MapLoader* loader() const;
    [[nodiscard]] View* view() const;

    [[nodiscard]] QString styleSheetName() const;
    void setStylesheetName(const QString &styleSheet);

    [[nodiscard]] FloorLevelModel *floorLevelModel() const;

    [[nodiscard]] Q_INVOKABLE KOSMIndoorMap::OSMElement elementAt(double x, double y) const;

    [[nodiscard]] bool hasError() const;
    [[nodiscard]] QString errorMessage() const;

    [[nodiscard]] QString region() const;
    void setRegion(const QString &region);
    [[nodiscard]] QString timeZoneId() const;
    void setTimeZoneId(const QString &tz);

Q_SIGNALS:
    void mapDataChanged();
    void styleSheetChanged();
    void currentFloorLevelChanged();
    void errorChanged();
    void overlaySourcesChanged();
    void regionChanged();
    void timeZoneChanged();
    void hoveredElementChanged();

protected:
    void geometryChange(const QRectF &newGeometry, const QRectF &oldGeometry) override;

private:
    void clear();
    void loaderDone();
    [[nodiscard]] MapData mapData() const;
    [[nodiscard]] QVariant overlaySources() const;
    void setOverlaySources(const QVariant &overlays);

    void addOverlaySource(std::vector<QPointer<AbstractOverlaySource>> &overlaySources, const QVariant &source);
    void overlayUpdate();
    void overlayReset();

    [[nodiscard]] OSMElement hoveredElement() const;
    void setHoveredElement(const OSMElement &element);

    MapLoader *m_loader = nullptr;
    MapData m_data;
    SceneGraph m_sg;
    View *m_view = nullptr;
    QUrl m_styleSheetUrl;
    MapCSSLoader *m_styleLoader = nullptr;
    MapCSSStyle m_style;
    SceneController m_controller;
    PainterRenderer m_renderer;
    FloorLevelModel *m_floorLevelModel = nullptr;
    QString m_errorMessage;
    QVariant m_overlaySources;
    std::vector<std::unique_ptr<AbstractOverlaySource>> m_ownedOverlaySources;
};

}

#endif // KOSMINDOORMAP_MAPITEM_H
