/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KOSMINDOORMAP_TILECACHE_P_H
#define KOSMINDOORMAP_TILECACHE_P_H

#include "kosmindoormap_export.h"

#include <KOSMIndoorMap/NetworkAccessManagerFactory>

#include <QDateTime>
#include <QFile>
#include <QObject>

#include <deque>

class QNetworkAccessManager;
class QNetworkReply;

namespace OSM {
class BoundingBox;
class Coordinate;
}

namespace KOSMIndoorMap {

/** Identifier of a slippy map tile.
 *  @see https://wiki.openstreetmap.org/wiki/Slippy_map_tilenames
 *  @internal only exported for unit tests
 */
class KOSMINDOORMAP_EXPORT Tile
{
public:
    inline Tile() = default;
    explicit inline Tile(uint32_t _x, uint32_t _y, uint8_t _z)
        : x(_x) , y(_y) , z(_z) {}

    static Tile fromCoordinate(double lat, double lon, uint8_t z);

    OSM::Coordinate topLeft() const;
    OSM::BoundingBox boundingBox() const;

    // move up and down the z hierarchy for the same location
    Tile topLeftAtZ(uint8_t z) const;
    Tile bottomRightAtZ(uint8_t z) const;

    uint32_t x = 0;
    uint32_t y = 0;
    uint8_t z = 0;
    QDateTime ttl;
};

/** OSM vector tile downloading and cache management. */
class TileCache : public QObject
{
    Q_OBJECT
public:
    explicit TileCache(const NetworkAccessManagerFactory &namFactory,  QObject *parent = nullptr);
    ~TileCache();

    /** Returns the path to the cached content of @p tile, if present locally. */
    QString cachedTile(Tile tile) const;

    /** Ensure @p tile is locally cached. */
    void ensureCached(Tile tile);

    /** Triggers the download of tile @p tile. */
    void downloadTile(Tile tile);

    /** Number of pending downloads. */
    int pendingDownloads() const;

    /** Cancel all pending downloads. */
    void cancelPending();

    /** Expire old cached tiles. */
    void expire();

Q_SIGNALS:
    void tileLoaded(Tile tile);
    void tileError(Tile tile, const QString &errorMessage);

private:
    QString cachePath(Tile tile) const;
    void downloadNext();
    void dataReceived(QNetworkReply *reply);
    void downloadFinished(QNetworkReply *reply, Tile tile);
    void updateTtl(const QString &filePath, const QDateTime &ttl);

    NetworkAccessManagerFactory m_nam;
    QFile m_output;
    std::deque<Tile> m_pendingDownloads;
};

}

#endif // KOSMINDOORMAP_TILECACHE_P_H
