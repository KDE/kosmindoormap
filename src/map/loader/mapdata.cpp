/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <config-kosmindoormap.h>
#include "mapdata.h"
#include "levelparser_p.h"

#if !BUILD_TOOLS_ONLY
#include "style/mapcssdeclaration_p.h"
#include "style/mapcssresult.h"
#include "style/mapcssstate_p.h"

#include <KOSMIndoorMap/MapCSSParser>
#include <KOSMIndoorMap/MapCSSProperty>
#include <KOSMIndoorMap/MapCSSStyle>
#endif

#include <osm/geomath.h>

#include <QPointF>
#include <QTimeZone>

using namespace KOSMIndoorMap;

MapLevel::MapLevel(int level)
    : m_level(level)
{
}

MapLevel::~MapLevel() = default;

bool MapLevel::operator<(const MapLevel &other) const
{
    return m_level > other.m_level;
}

bool MapLevel::operator==(const MapLevel &other) const
{
    return m_level == other.m_level;
}

bool MapLevel::hasName() const
{
    return !m_levelName.isEmpty();
}

QString MapLevel::name() const
{
    if (m_levelName.isEmpty()) {
        return QString::number(m_level / 10);
    }
    return m_levelName;
}

void MapLevel::setName(const QString &name)
{
    m_levelName = name;
}

bool MapLevel::isFullLevel() const
{
    return m_level % 10 == 0;
}

int MapLevel::fullLevelBelow() const
{
    return m_level < 0 ? (m_level - (10 + m_level % 10)) : (m_level - m_level % 10);
}

int MapLevel::fullLevelAbove() const
{
    return m_level < 0 ? (m_level - m_level % 10) : (m_level + (10 - m_level % 10));
}

int MapLevel::numericLevel() const
{
    return m_level;
}

namespace KOSMIndoorMap {
class MapDataPrivate {
public:
    OSM::DataSet m_dataSet;
    OSM::BoundingBox m_bbox;

    OSM::TagKey m_levelRefTag;
    OSM::TagKey m_nameTag;

    std::map<MapLevel, std::vector<OSM::Element>> m_levelMap;
    std::map<MapLevel, std::size_t> m_dependentElementCounts;

    QString m_regionCode;
    QTimeZone m_timeZone;
};
}

MapData::MapData()
    : d(std::make_shared<MapDataPrivate>())
{
}

MapData::MapData(const MapData&) = default;
MapData::MapData(MapData&&) = default;
MapData::~MapData() = default;

MapData& MapData::operator=(const MapData&) = default;
MapData& MapData::operator=(MapData&&) = default;

const OSM::DataSet& MapData::dataSet() const
{
    return d->m_dataSet;
}

bool MapData::isEmpty() const
{
    return !d || d->m_levelMap.empty();
}

bool MapData::operator==(const MapData &other) const
{
    return d == other.d;
}

OSM::DataSet& MapData::dataSet()
{
    return d->m_dataSet;
}

void MapData::setDataSet(OSM::DataSet &&dataSet)
{
    d->m_dataSet = std::move(dataSet);

    d->m_levelRefTag = d->m_dataSet.tagKey("level:ref");
    d->m_nameTag = d->m_dataSet.tagKey("name");

    d->m_levelMap.clear();
    d->m_bbox = {};

    processElements();
    filterLevels();
}

OSM::BoundingBox MapData::boundingBox() const
{
    return d->m_bbox;
}

void MapData::setBoundingBox(OSM::BoundingBox bbox)
{
    d->m_bbox = bbox;
}

const std::map<MapLevel, std::vector<OSM::Element>>& MapData::levelMap() const
{
    return d->m_levelMap;
}

void MapData::processElements()
{
    const auto levelTag = d->m_dataSet.tagKey("level");
    const auto repeatOnTag = d->m_dataSet.tagKey("repeat_on");
    const auto buildingLevelsTag = d->m_dataSet.tagKey("building:levels");
    const auto buildingMinLevelTag = d->m_dataSet.tagKey("building:min_level");
    const auto buildingLevelsUndergroundTag = d->m_dataSet.tagKey("building:levels:underground");
    const auto maxLevelTag = d->m_dataSet.tagKey("max_level");
    const auto minLevelTag = d->m_dataSet.tagKey("min_level");
    const auto countryTag = d->m_dataSet.tagKey("addr:country");

#if !BUILD_TOOLS_ONLY
    MapCSSParser p;
    auto filter = p.parse(QStringLiteral(":/org.kde.kosmindoormap/assets/css/input-filter.mapcss"));
    if (p.hasError()) {
        qWarning() << p.errorMessage();
    }
    filter.compile(d->m_dataSet);
    MapCSSResult filterResult;
#endif

    OSM::for_each(d->m_dataSet, [&](auto e) {
        // discard everything here that is tag-less (and thus likely part of a higher-level geometry)
        if (!e.hasTags()) {
            return;
        }

        // attempt to detect the country we are in
        if (d->m_regionCode.isEmpty()) {
            const auto countryCode = e.tagValue(countryTag);
            if (countryCode.size() == 2 && std::isupper(static_cast<unsigned char>(countryCode[0])) && std::isupper(static_cast<unsigned char>(countryCode[1]))) {
                d->m_regionCode = QString::fromUtf8(countryCode);
            }
        }

        // apply the input filter, anything that explicitly got opacity 0 will be discarded
        bool isDependentElement = false;
#if !BUILD_TOOLS_ONLY
        MapCSSState filterState;
        filterState.element = e;
        filter.evaluate(std::move(filterState), filterResult);
        if (auto prop = filterResult[{}].declaration(MapCSSProperty::Opacity)) {
            if (prop->doubleValue() == 0.0) {
                qDebug() << "input filter dropped" << e.url();
                return;
            }
            // anything that doesn't work on its own is a "dependent element"
            // we discard levels only containing dependent elements, but we retain all of them if the
            // level contains an element we are sure about that we can display it
            if (prop->doubleValue() < 1.0) {
                isDependentElement = true;
            }
        }
#endif

        // bbox computation
        e.recomputeBoundingBox(d->m_dataSet);
        d->m_bbox = OSM::unite(e.boundingBox(), d->m_bbox);

        // multi-level building element
        // we handle this first, before level=, as level is often used instead
        // of building:min_level in combination with building:level
        const auto buildingLevels = e.tagValue(buildingLevelsTag, maxLevelTag).toInt();
        if (buildingLevels > 0) {
            const auto startLevel = e.tagValue(buildingMinLevelTag, levelTag, minLevelTag).toInt();
            //qDebug() << startLevel << buildingLevels << e.url();
            for (auto i = startLevel; i < startLevel + buildingLevels; ++i) {
                addElement(i * 10, e, true);
            }
        }
        const auto undergroundLevels = e.tagValue(buildingLevelsUndergroundTag).toUInt();
        for (auto i = undergroundLevels; i > 0; --i) {
            addElement(-i * 10, e, true);
        }
        if (buildingLevels > 0 || undergroundLevels > 0) {
            return;
        }

        // element with explicit level specified
        auto level = e.tagValue(levelTag);
        auto repeatOn = e.tagValue(repeatOnTag);
        if (level.isEmpty() && repeatOn.isEmpty()) {
            // no level information available
            d->m_levelMap[MapLevel{}].push_back(e);
            if (isDependentElement) {
                d->m_dependentElementCounts[MapLevel{}]++;
            }
        } else {
            LevelParser::parse(std::move(level), e, [this, isDependentElement](int level, OSM::Element e) {
                addElement(level, e, isDependentElement);
            });
            LevelParser::parse(std::move(repeatOn), e, [this, isDependentElement](int level, OSM::Element e) {
                addElement(level, e, isDependentElement);
            });
        }
    });
}

void MapData::addElement(int level, OSM::Element e, bool isDependentElement)
{
    MapLevel l(level);
    auto it = d->m_levelMap.find(l);
    if (it == d->m_levelMap.end()) {
        l.setName(levelName(e));
        d->m_levelMap[l] = {e};
    } else {
        if (!(*it).first.hasName()) {
            // name does not influence op< behavior, so modifying the key here is safe
            const_cast<MapLevel&>((*it).first).setName(levelName(e));
        }
        (*it).second.push_back(e);
    }
    if (isDependentElement) {
        d->m_dependentElementCounts[l]++;
    }
}

static bool isPlausibleLevelName(const QByteArray &s)
{
    return !s.isEmpty() && !s.contains(';');
}

QString MapData::levelName(OSM::Element e)
{
    const auto n = e.tagValue(d->m_levelRefTag);
    if (isPlausibleLevelName(n)) {
        return QString::fromUtf8(n);
    }

    if (e.type() == OSM::Type::Relation) {
        const auto isLevelRel = std::all_of(e.relation()->members.begin(), e.relation()->members.end(), [](const auto &mem) {
            return std::strcmp(mem.role().name(), "shell") == 0 || std::strcmp(mem.role().name(), "buildingpart") == 0;
        });
        if (isLevelRel) {
            const auto n = e.tagValue(d->m_nameTag);
            if (isPlausibleLevelName(n)) {
                return QString::fromUtf8(n);
            }
        }
    }

    return {};
}

void MapData::filterLevels()
{
    // remove all levels that don't contain something we are sure would make a meaningful output
    // always retain the base level though
    for (auto it = d->m_levelMap.begin(); it != d->m_levelMap.end();) {
        if ((*it).first.numericLevel() != 0 && d->m_dependentElementCounts[(*it).first] == (*it).second.size()) {
            it = d->m_levelMap.erase(it);
        } else {
            ++it;
        }
    }
    d->m_dependentElementCounts.clear();
}

QPointF MapData::center() const
{
    return QPointF(d->m_bbox.center().lonF(), d->m_bbox.center().latF());
}

float MapData::radius() const
{
    return std::max(OSM::distance(d->m_bbox.center(), d->m_bbox.min), OSM::distance(d->m_bbox.center(), d->m_bbox.max));
}

QString MapData::regionCode() const
{
    return d->m_regionCode;
}

void MapData::setRegionCode(const QString &regionCode)
{
    d->m_regionCode = regionCode;
}

QTimeZone MapData::timeZone() const
{
    return d->m_timeZone;
}

void MapData::setTimeZone(const QTimeZone &tz)
{
    d->m_timeZone = tz;
}

QString MapData::timeZoneId() const
{
    return QString::fromUtf8(d->m_timeZone.id());
}

#include "moc_mapdata.cpp"
