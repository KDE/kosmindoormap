/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KOSMINDOORMAP_MAPLOADER_H
#define KOSMINDOORMAP_MAPLOADER_H

#include "kosmindoormap_export.h"

#include <QObject>

#include <memory>

namespace OSM {
class BoundingBox;
}

/** OSM-based multi-floor indoor maps for buildings. */
namespace KOSMIndoorMap {

class MapData;
class MapLoaderPrivate;
class Tile;

/** Loader for OSM data for a single station or airport. */
class KOSMINDOORMAP_EXPORT MapLoader : public QObject
{
    Q_OBJECT
    /** Indicates we are downloading content. Use for progress display. */
    Q_PROPERTY(bool isLoading READ isLoading NOTIFY isLoadingChanged)
public:
    explicit MapLoader(QObject *parent = nullptr);
    ~MapLoader();

    /** Load a single O5M or OSM PBF file. */
    Q_INVOKABLE void loadFromFile(const QString &fileName);
    /** Load map for the given coordinates.
     *  This can involve online access.
     */
    Q_INVOKABLE void loadForCoordinate(double lat, double lon);
    /** Same as the above, but ensureing the requested data is cached until @p ttl. */
    void loadForCoordinate(double lat, double lon, const QDateTime &ttl);

    /** Load map data for the given bounding box, without applying the boundary search. */
    void loadForBoundingBox(OSM::BoundingBox box);
    /** QML-compatible overload of the above. */
    Q_INVOKABLE void loadForBoundingBox(double minLat, double minLon, double maxLat, double maxLon);

    /** Load map data for the given tile. */
    void loadForTile(Tile tile);

    /** Add a changeset to be applied on top of the data loaded by any of the load() methods.
     *  Needs to be called after any of the load methods and before returning to the event loop.
     *  @param url can be a local file or a HTTP URL which is downloaded if needed.
     */
    Q_INVOKABLE void addChangeSet(const QUrl &url);

    /** Take out the completely loaded result.
     *  Do this before loading the next map with the same loader.
     */
    MapData&& takeData();

    [[nodiscard]] bool isLoading() const;

    [[nodiscard]] bool hasError() const;
    [[nodiscard]] QString errorMessage() const;

Q_SIGNALS:
    /** Emitted when the requested data has been loaded. */
    void done();
    void isLoadingChanged();

private:
    void downloadTiles();
    void downloadFinished();
    void downloadFailed(Tile tile, const QString &errorMessage);
    void loadTiles();
    [[nodiscard]] Tile makeTile(uint32_t x, uint32_t y) const;
    void applyNextChangeSet();
    void applyChangeSet(const QUrl &url, QIODevice *io);

    std::unique_ptr<MapLoaderPrivate> d;
};

}

#endif // KOSMINDOORMAP_MAPLOADER_H
