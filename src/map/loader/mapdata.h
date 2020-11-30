/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KOSMINDOORMAP_MAPDATA_H
#define KOSMINDOORMAP_MAPDATA_H

#include "kosmindoormap_export.h"

#include <KOSM/Datatypes>
#include <KOSM/Element>

#include <QMetaType>

#include <map>
#include <memory>
#include <vector>

class QPointF;
class QTimeZone;

namespace KOSMIndoorMap {

/** A floor level. */
class KOSMINDOORMAP_EXPORT MapLevel
{
public:
    explicit MapLevel(int level = 0);
    ~MapLevel();

    bool operator<(const MapLevel &other) const;
    bool operator==(const MapLevel &other) const;

    bool hasName() const;
    QString name() const;
    void setName(const QString &name);

    bool isFullLevel() const;
    /** In case this is not a full level, this returns the numeric values of the full levels above/below. */
    int fullLevelBelow() const;
    int fullLevelAbove() const;

    int numericLevel() const;

private:
    int m_level = 0;
    QString m_levelName;
};
}

Q_DECLARE_METATYPE(KOSMIndoorMap::MapLevel)

namespace KOSMIndoorMap {
class MapDataPrivate;

/** Raw OSM map data, separated by levels. */
class KOSMINDOORMAP_EXPORT MapData
{
    Q_GADGET
    /** Center position of the bounding box for QML usage (longitude/latitude, in degree). */
    Q_PROPERTY(QPointF center READ center)
    /** Radius from the bounding box center encompassing the entire bounding box, in meters.
     *  Useful for circular search queries.
     */
    Q_PROPERTY(float radius READ radius)

    Q_PROPERTY(QString regionCode READ regionCode)
    Q_PROPERTY(QString timeZone READ timeZoneId)
public:
    explicit MapData();
    MapData(const MapData&);
    MapData(MapData&&);
    ~MapData();

    MapData& operator=(const MapData&);
    MapData& operator=(MapData&&);

    bool isEmpty() const;
    bool operator==(const MapData &other) const;

    const OSM::DataSet& dataSet() const;
    OSM::DataSet& dataSet();
    void setDataSet(OSM::DataSet &&dataSet);

    OSM::BoundingBox boundingBox() const;
    void setBoundingBox(OSM::BoundingBox bbox);

    const std::map<MapLevel, std::vector<OSM::Element>>& levelMap() const;

    QPointF center() const;
    float radius() const;

    /** ISO 3166-1/2 region or country code of the area covered by this map data. */
    QString regionCode() const;
    void setRegionCode(const QString &regionCode);

    /** Timezone the are covered by this map data is in. */
    QTimeZone timeZone() const;
    void setTimeZone(const QTimeZone &tz);

private:
    void processElements();
    void addElement(int level, OSM::Element e, bool isDependentElement);
    QString levelName(OSM::Element e);
    void filterLevels();

    QString timeZoneId() const;

    std::shared_ptr<MapDataPrivate> d;
};

}

Q_DECLARE_METATYPE(KOSMIndoorMap::MapData)

#endif // KOSMINDOORMAP_MAPDATA_H
