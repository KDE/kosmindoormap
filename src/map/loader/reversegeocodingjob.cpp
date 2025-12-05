/*
    SPDX-FileCopyrightText: 2025 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "reversegeocodingjob.h"

#include "logging.h"
#include "mapdata.h"
#include "maploader.h"

#include <osm/geomath.h>

namespace KOSMIndoorMap {
class ReverseGeocodingJobPrivate {
public:
    double lat = NAN;
    double lon = NAN;
    double radius = 1.0;
    NetworkAccessManagerFactory m_nam = KOSMIndoorMap::defaultNetworkAccessManagerFactory;
    MapLoader loader;
    MapData mapData;
    std::vector<OSM::Element> result;
};
}

using namespace KOSMIndoorMap;

ReverseGeocodingJob::ReverseGeocodingJob(QObject *parent)
    : QObject(parent)
    , d(std::make_unique<ReverseGeocodingJobPrivate>())
{
    connect(&d->loader, &MapLoader::done, this, &ReverseGeocodingJob::mapLoadingDone);
}

ReverseGeocodingJob::~ReverseGeocodingJob() = default;

void ReverseGeocodingJob::setNetworkAccessManagerFactory(const NetworkAccessManagerFactory &namFactory)
{
    d->m_nam = namFactory;
}

void ReverseGeocodingJob::setCoordinate(double lat, double lon)
{
    d->lat = lat;
    d->lon = lon;
}

void ReverseGeocodingJob::setRadius(double radius)
{
    d->radius = radius;
}

void ReverseGeocodingJob::start()
{
    qCDebug(Log);

    const auto dlat = d->radius / OSM::distance(d->lat, 0.0, d->lat, 1.0);
    const auto dlon = d->radius / OSM::distance(0.0, d->lon, 1.0, d->lon);
    d->loader.loadForBoundingBox(d->lat - dlat, d->lon - dlon, d->lat + dlat, d->lon + dlon);
}

bool ReverseGeocodingJob::hasError() const
{
    return d->loader.hasError();
}

QString ReverseGeocodingJob::errorMessage() const
{
    return d->loader.errorMessage();
}

const std::vector<OSM::Element>& ReverseGeocodingJob::result() const
{
    return d->result;
}

std::vector<OSM::Element>&& ReverseGeocodingJob::takeResult()
{
    return std::move(d->result);
}

const MapData& ReverseGeocodingJob::mapData() const
{
    return d->mapData;
}

MapData&& ReverseGeocodingJob::takeMapData()
{
    return std::move(d->mapData);
}

void ReverseGeocodingJob::mapLoadingDone()
{
    qCDebug(Log) << d->loader.hasError() << d->loader.errorMessage();
    if (d->loader.hasError()) {
        Q_EMIT finished();
        return;
    }

    d->mapData = std::move(d->loader.takeData());
    const auto nameKey = d->mapData.dataSet().tagKey("name");

    struct Result {
        OSM::Element elem;
        double distance;
        double area;
    };
    std::vector<Result> r;

    // filter
    OSM::for_each(d->mapData.dataSet(), [this, nameKey, &r](auto elem) {
        if (!elem.hasTags() || !elem.hasTag(nameKey)) {
            return;
        }

        const auto bbox = elem.boundingBox();
        const auto dist = OSM::distance(OSM::Coordinate(d->lat, d->lon), elem.center());
        if ((elem.type() == OSM::Type::Node || !OSM::contains(bbox, OSM::Coordinate(d->lat, d->lon))) && dist > d->radius) {
            return;
        }

        r.push_back({
            .elem = elem,
            .distance = dist,
            .area = bbox.widthF() * bbox.heightF()
        });
    });

    // sort
    std::ranges::sort(r, [](const auto &lhs, const auto &rhs) {
       if (lhs.distance == rhs.distance) {
           if (lhs.area == rhs.area) {
               return lhs.elem.type() > rhs.elem.type();
            }
           return lhs.area < rhs.area;
        }
        return lhs.distance < rhs.distance;
    });

    d->result.reserve(r.size());
    std::ranges::transform(r, std::back_inserter(d->result), [](const auto &r) { return r.elem; });

    qCDebug(Log) << d->result.size();
    Q_EMIT finished();
}
