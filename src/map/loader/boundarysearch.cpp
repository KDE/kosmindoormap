/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "boundarysearch_p.h"

#include <osm/datatypes.h>
#include <osm/element.h>
#include <osm/geomath.h>

#include <QDebug>

using namespace KOSMIndoorMap;

enum {
    BoundingBoxMargin = 100, // margin around the final result, in meters
    BoundingBoxMaxSize = 2000, // maximum width/height of the bounding box, in meters
};

void BoundarySearch::init(OSM::Coordinate coord)
{
    m_center = coord;

    m_bbox = {coord, coord};
    m_relevantIds.clear();
}

static OSM::Id actualId(OSM::Element e, OSM::TagKey mxoidTag)
{
    const auto mxoid = e.tagValue(mxoidTag);
    if (!mxoid.isEmpty()) {
        return mxoid.toLongLong();
    }
    return e.id();
}

static bool isPolygon(OSM::Element e)
{
    return (e.type() == OSM::Type::Way && e.way()->isClosed())
        || (e.type() == OSM::Type::Relation && e.tagValue("type") == "multipolygon");
}

void BoundarySearch::resolveTagKeys(const OSM::DataSet& dataSet)
{
    // cache tag keys for fast lookup
    m_tag.mxoid = dataSet.tagKey("mx:oid");
    m_tag.building = dataSet.tagKey("building");
    m_tag.railway = dataSet.tagKey("railway");
    m_tag.aeroway = dataSet.tagKey("aeroway");
    m_tag.public_transport = dataSet.tagKey("public_transport");
    m_tag.amenity = dataSet.tagKey("amenity");
    m_tag.tourism =  dataSet.tagKey("tourism");
    m_tag.leisure = dataSet.tagKey("leisure");
    m_tag.shop = dataSet.tagKey("shop");
}

bool BoundarySearch::isRelevantPolygon(const OSM::Element e) const
{
    if (!isPolygon(e) || !OSM::contains(e.boundingBox(), m_center)) {
        return false;
    }

    return !e.tagValue(m_tag.amenity).isEmpty()
        || !e.tagValue(m_tag.tourism).isEmpty()
        || !e.tagValue(m_tag.leisure).isEmpty()
        || !e.tagValue(m_tag.shop).isEmpty();
}

/* There's a number of critieria being considered here:
 * - a certain minimum radius around center (see BoundingBoxMargin)
 * - an upper limit (BoundingBoxMaxSize), to avoid this growing out of control
 * - buildings, stations or airports containing the center position
 * -- for now with manual geometry re-assmbly from Marble vector tiles, ideally this will happen in a general step beforehand
 * - relevant elements (e.g. platforms or terminal buildings) in the vicinity of center (TODO)
 */
OSM::BoundingBox BoundarySearch::boundingBox(const OSM::DataSet& dataSet)
{
    resolveTagKeys(dataSet);

    if (m_relevantIds.empty()) { // first pass over the center tile
        OSM::for_each(dataSet, [this, &dataSet](OSM::Element e) {
            if (!e.boundingBox().isValid()) {
                e.recomputeBoundingBox(dataSet);
            }

            const bool isRelevant = !e.tagValue(m_tag.building).isEmpty()
                || !e.tagValue(m_tag.railway).isEmpty()
                || !e.tagValue(m_tag.aeroway).isEmpty()
                || isRelevantPolygon(e);

            if (!isRelevant) {
                return;
            }

            m_relevantIds.insert(actualId(e, m_tag.mxoid));
            m_bbox = OSM::unite(m_bbox, e.boundingBox());
        }, OSM::IncludeRelations | OSM::IncludeWays);
    }

    OSM::BoundingBox bbox = m_bbox;
    OSM::for_each(dataSet, [&](OSM::Element e) {
        const auto railwayValue = e.tagValue(m_tag.railway);
        const bool isStation = railwayValue == "station"
            || railwayValue == "platform"
            || e.tagValue(m_tag.building) == "train_station"
            || e.tagValue(m_tag.public_transport) == "platform";
        const bool isAirport = (e.tagValue(m_tag.aeroway) == "aerodrome");
        const bool isRelevant = isRelevantPolygon(e);
        if (!isStation && !isAirport && !isRelevant) {
            return;
        }

        e.recomputeBoundingBox(dataSet); // unconditionally as this obviously grows as we load more data

        if (m_relevantIds.count(actualId(e, m_tag.mxoid))) {
            m_bbox = OSM::unite(m_bbox, e.boundingBox());
            bbox = OSM::unite(m_bbox, bbox);
        } else if (!isRelevant && OSM::intersects(e.boundingBox(), m_bbox)) {
            bbox = OSM::unite(bbox, e.boundingBox());
        }
    }, OSM::IncludeRelations | OSM::IncludeWays);

    return clampBoundingBox(growBoundingBox(bbox, BoundingBoxMargin), BoundingBoxMaxSize);
}

OSM::BoundingBox BoundarySearch::growBoundingBox(const OSM::BoundingBox &bbox, double meters) const
{
    const auto dlon = meters / OSM::distance(m_center.latF(), 0.0, m_center.latF(), 1.0);
    const auto dlat = meters / OSM::distance(0.0, m_center.lonF(), 1.0, m_center.lonF());
    return OSM::BoundingBox(OSM::Coordinate(bbox.min.latF() - dlat, bbox.min.lonF() - dlon),
                            OSM::Coordinate(bbox.max.latF() + dlat, bbox.max.lonF() + dlon));
}

OSM::BoundingBox BoundarySearch::clampBoundingBox(const OSM::BoundingBox &bbox, double meters) const
{
    // TODO don'T do this around the bbox center, but biased towards m_center
    const auto dlon = std::max(0.0, bbox.widthF() - (meters / OSM::distance(m_center.latF(), 0.0, m_center.latF(), 1.0))) / 2.0;
    const auto dlat = std::max(0.0, bbox.heightF() - (meters / OSM::distance(0.0, m_center.lonF(), 1.0, m_center.lonF()))) / 2.0;
    return OSM::BoundingBox(OSM::Coordinate(bbox.min.latF() + dlat, bbox.min.lonF() + dlon),
                            OSM::Coordinate(bbox.max.latF() - dlat, bbox.max.lonF() - dlon));
}
