/*
    SPDX-FileCopyrightText: 2025 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KOSMINDOORMAP_REVERSEGEOCODINGJOB_H
#define KOSMINDOORMAP_REVERSEGEOCODINGJOB_H

#include "kosmindoormap_export.h"

#include <KOSMIndoorMap/NetworkAccessManagerFactory>

#include <QObject>

#include <memory>

namespace OSM {
class Element;
}

namespace KOSMIndoorMap {

class MapData;
class ReverseGeocodingJobPrivate;

/** Reverse geocoding using the OSM raw data tiles.
 *  @since 26.04
 */
class KOSMINDOORMAP_EXPORT ReverseGeocodingJob : public QObject
{
    Q_OBJECT
public:
    explicit ReverseGeocodingJob(QObject *parent = nullptr);
    ~ReverseGeocodingJob();

    void setNetworkAccessManagerFactory(const NetworkAccessManagerFactory &namFactory);
    /** Coordinate and radius to search for. */
    void setCoordinate(double lat, double lon);
    void setRadius(double radius);
    void start();

    [[nodiscard]] bool hasError() const;
    [[nodiscard]] QString errorMessage() const;

    /** All OSM elements within the search radius, ordered by proximity to the center coordinate. */
    [[nodiscard]] const std::vector<OSM::Element>& result() const;
    [[nodiscard]] std::vector<OSM::Element>&& takeResult();

    /** Map data the results are from. Keep alive as long as you keep the result elements around. */
    [[nodiscard]] const MapData& mapData() const;
    [[nodiscard]] MapData&& takeMapData();

Q_SIGNALS:
    void finished();

private:
    Q_DECL_HIDDEN void mapLoadingDone();

    std::unique_ptr<ReverseGeocodingJobPrivate> d;
};

}

#endif // KOSMINDOORMAP_MAPLOADER_H
