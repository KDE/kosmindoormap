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

private:
    void processElements();
    void addElement(int level, OSM::Element e, bool isDependentElement);
    QString levelName(OSM::Element e);
    void filterLevels();

    std::shared_ptr<MapDataPrivate> d;
};

}

Q_DECLARE_METATYPE(KOSMIndoorMap::MapData)

#endif // KOSMINDOORMAP_MAPDATA_H
