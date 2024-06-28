/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <config-kosmindoormap.h>

#include "maploader.h"
#include "boundarysearch_p.h"
#include "logging.h"
#include "mapdata.h"
#include "marblegeometryassembler_p.h"
#include "tilecache_p.h"

#include "network/useragent_p.h"

#include <osm/datatypes.h>
#include <osm/datasetmergebuffer.h>
#include <osm/element.h>
#include <osm/o5mparser.h>
#include <osm/io.h>

#include <QDateTime>
#include <QElapsedTimer>
#include <QFile>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QRect>
#include <QUrl>

#include <deque>

using namespace Qt::Literals::StringLiterals;

enum {
    TileZoomLevel = 17
};

inline void initResources()  // needs to be outside of a namespace
{
#if !BUILD_TOOLS_ONLY
    Q_INIT_RESOURCE(assets);
#endif
}

namespace KOSMIndoorMap {
class MapLoaderPrivate {
public:
    NetworkAccessManagerFactory m_nam = KOSMIndoorMap::defaultNetworkAccessManagerFactory; // TODO make externally configurable
    OSM::DataSet m_dataSet;
    OSM::DataSetMergeBuffer m_mergeBuffer;
    MarbleGeometryAssembler m_marbleMerger;
    MapData m_data;
    TileCache m_tileCache{m_nam};
    OSM::BoundingBox m_tileBbox;
    OSM::BoundingBox m_targetBbox;
    QRect m_loadedTiles;
    std::vector<Tile> m_pendingTiles;
    std::unique_ptr<BoundarySearch> m_boundarySearcher;
    QDateTime m_ttl;
    std::deque<QUrl> m_pendingChangeSets;

    QString m_errorMessage;
};
}

using namespace KOSMIndoorMap;

MapLoader::MapLoader(QObject *parent)
    : QObject(parent)
    , d(new MapLoaderPrivate)
{
    initResources();
    connect(&d->m_tileCache, &TileCache::tileLoaded, this, &MapLoader::downloadFinished);
    connect(&d->m_tileCache, &TileCache::tileError, this, &MapLoader::downloadFailed);
    d->m_tileCache.expire();
}

MapLoader::~MapLoader() = default;

void MapLoader::loadFromFile(const QString &fileName)
{
    QElapsedTimer loadTime;
    loadTime.start();

    d->m_errorMessage.clear();
    QFile f(fileName.contains(QLatin1Char(':')) ? QUrl::fromUserInput(fileName).toLocalFile() : fileName);
    if (!f.open(QFile::ReadOnly)) {
        qCritical() << f.fileName() << f.errorString();
        return;
    }
    const auto data = f.map(0, f.size());

    auto reader = OSM::IO::readerForFileName(fileName, &d->m_dataSet);
    if (!reader) {
        qCWarning(Log) << "no file reader for" << fileName;
        return;
    }
    reader->read(data, f.size());
    d->m_data = MapData();
    qCDebug(Log) << "o5m loading took" << loadTime.elapsed() << "ms";
    QMetaObject::invokeMethod(this, &MapLoader::applyNextChangeSet, Qt::QueuedConnection);
}

void MapLoader::loadForCoordinate(double lat, double lon)
{
    loadForCoordinate(lat, lon, {});
}

void MapLoader::loadForCoordinate(double lat, double lon, const QDateTime &ttl)
{
    d->m_ttl = ttl;
    d->m_tileBbox = {};
    d->m_targetBbox = {};
    d->m_pendingTiles.clear();
    d->m_boundarySearcher = std::make_unique<BoundarySearch>();
    d->m_boundarySearcher->init(OSM::Coordinate(lat, lon));
    d->m_errorMessage.clear();
    d->m_marbleMerger.setDataSet(&d->m_dataSet);
    d->m_data = MapData();

    auto tile = Tile::fromCoordinate(lat, lon, TileZoomLevel);
    d->m_loadedTiles = QRect(tile.x, tile.y, 1, 1);
    d->m_pendingTiles.push_back(std::move(tile));
    downloadTiles();
}

void MapLoader::loadForBoundingBox(OSM::BoundingBox box)
{
    d->m_ttl = {};
    d->m_tileBbox = box;
    d->m_targetBbox = box;
    d->m_pendingTiles.clear();
    d->m_errorMessage.clear();
    d->m_marbleMerger.setDataSet(&d->m_dataSet);
    d->m_data = MapData();

    const auto topLeftTile = Tile::fromCoordinate(box.min.latF(), box.min.lonF(), TileZoomLevel);
    const auto bottomRightTile = Tile::fromCoordinate(box.max.latF(), box.max.lonF(), TileZoomLevel);
    for (auto x = topLeftTile.x; x <= bottomRightTile.x; ++x) {
        for (auto y = bottomRightTile.y; y <= topLeftTile.y; ++y) {
            d->m_pendingTiles.push_back(makeTile(x, y));
        }
    }
    downloadTiles();
}

void MapLoader::loadForBoundingBox(double minLat, double minLon, double maxLat, double maxLon)
{
    loadForBoundingBox(OSM::BoundingBox(OSM::Coordinate{minLat, minLon}, OSM::Coordinate{maxLat, maxLon}));
}

void MapLoader::loadForTile(Tile tile)
{
    d->m_ttl = {};
    d->m_tileBbox = tile.boundingBox();
    d->m_targetBbox = {};
    d->m_pendingTiles.clear();
    d->m_errorMessage.clear();
    d->m_marbleMerger.setDataSet(&d->m_dataSet);
    d->m_data = MapData();

    if (tile.z >= TileZoomLevel) {
        d->m_pendingTiles.push_back(std::move(tile));
    } else {
        const auto start = tile.topLeftAtZ(TileZoomLevel);
        const auto end = tile.bottomRightAtZ(TileZoomLevel);
        for (auto x = start.x; x <= end.x; ++x) {
            for (auto y = start.y; y <= end.y; ++y) {
                d->m_pendingTiles.push_back(makeTile(x, y));
            }
        }
    }

    downloadTiles();
}

void MapLoader::addChangeSet(const QUrl &url)
{
    d->m_pendingChangeSets.push_back(url);
}

MapData&& MapLoader::takeData()
{
    return std::move(d->m_data);
}

void MapLoader::downloadTiles()
{
    for (const auto &tile : d->m_pendingTiles) {
        d->m_tileCache.ensureCached(tile);
    }
    if (d->m_tileCache.pendingDownloads() == 0) {
        // still go through the event loop when having everything cached already
        // this makes outside behavior more identical in both cases, and avoids
        // signal connection races etc.
        QMetaObject::invokeMethod(this, &MapLoader::loadTiles, Qt::QueuedConnection);
    } else {
        Q_EMIT isLoadingChanged();
    }
}

void MapLoader::downloadFinished()
{
    if (d->m_tileCache.pendingDownloads() > 0) {
        return;
    }
    loadTiles();
}

void MapLoader::loadTiles()
{
    QElapsedTimer loadTime;
    loadTime.start();

    OSM::O5mParser p(&d->m_dataSet);
    p.setMergeBuffer(&d->m_mergeBuffer);
    for (const auto &tile : d->m_pendingTiles) {
        const auto fileName = d->m_tileCache.cachedTile(tile);
        qCDebug(Log) << "loading tile" << fileName;
        QFile f(fileName);
        if (!f.open(QFile::ReadOnly)) {
            qWarning() << "Failed to open tile!" << f.fileName() << f.errorString();
            continue;
        }

        const auto data = f.map(0, f.size());
        if (!data) {
            qCritical() << "Failed to mmap tile!" << f.fileName() << f.size() << f.errorString();
            continue;
        }

        p.read(data, f.size());
        d->m_marbleMerger.merge(&d->m_mergeBuffer);

        d->m_tileBbox = OSM::unite(d->m_tileBbox, tile.boundingBox());
    }
    d->m_pendingTiles.clear();

    if (d->m_boundarySearcher) {
        const auto bbox = d->m_boundarySearcher->boundingBox(d->m_dataSet);
        qCDebug(Log) << "needed bbox:" << bbox << "got:" << d->m_tileBbox << d->m_loadedTiles;

        // expand left and right
        if (bbox.min.longitude < d->m_tileBbox.min.longitude) {
            d->m_loadedTiles.setLeft(d->m_loadedTiles.left() - 1);
            for (int y = d->m_loadedTiles.top(); y <= d->m_loadedTiles.bottom(); ++y) {
                d->m_pendingTiles.push_back(makeTile(d->m_loadedTiles.left(), y));
            }
        }
        if (bbox.max.longitude > d->m_tileBbox.max.longitude) {
            d->m_loadedTiles.setRight(d->m_loadedTiles.right() + 1);
            for (int y = d->m_loadedTiles.top(); y <= d->m_loadedTiles.bottom(); ++y) {
                d->m_pendingTiles.push_back(makeTile(d->m_loadedTiles.right(), y));
            }
        }

        // expand top/bottom: note that geographics and slippy map tile coordinates have a different understanding on what is "top"
        if (bbox.max.latitude > d->m_tileBbox.max.latitude) {
            d->m_loadedTiles.setTop(d->m_loadedTiles.top() - 1);
            for (int x = d->m_loadedTiles.left(); x <= d->m_loadedTiles.right(); ++x) {
                d->m_pendingTiles.push_back(makeTile(x, d->m_loadedTiles.top()));
            }
        }
        if (bbox.min.latitude < d->m_tileBbox.min.latitude) {
            d->m_loadedTiles.setBottom(d->m_loadedTiles.bottom() + 1);
            for (int x = d->m_loadedTiles.left(); x <= d->m_loadedTiles.right(); ++x) {
                d->m_pendingTiles.push_back(makeTile(x, d->m_loadedTiles.bottom()));
            }
        }

        if (!d->m_pendingTiles.empty()) {
            downloadTiles();
            return;
        }
        d->m_targetBbox = bbox;
    }

    d->m_marbleMerger.finalize();
    if (d->m_targetBbox.isValid()) {
        d->m_data.setBoundingBox(d->m_targetBbox);
    }
    d->m_boundarySearcher.reset();

    qCDebug(Log) << "o5m loading took" << loadTime.elapsed() << "ms";
    applyNextChangeSet();
}

Tile MapLoader::makeTile(uint32_t x, uint32_t y) const
{
    auto tile = Tile(x, y, TileZoomLevel);
    tile.ttl = d->m_ttl;
    return tile;
}

void MapLoader::downloadFailed(Tile tile, const QString& errorMessage)
{
    Q_UNUSED(tile);
    d->m_errorMessage = errorMessage;
    d->m_tileCache.cancelPending();
    Q_EMIT isLoadingChanged();
    Q_EMIT done();
}

bool MapLoader::isLoading() const
{
    return d->m_tileCache.pendingDownloads() > 0 || !d->m_pendingChangeSets.empty();
}

bool MapLoader::hasError() const
{
    return !d->m_errorMessage.isEmpty();
}

QString MapLoader::errorMessage() const
{
    return d->m_errorMessage;
}

void MapLoader::applyNextChangeSet()
{
    if (d->m_pendingChangeSets.empty() || hasError()) {
        d->m_data.setDataSet(std::move(d->m_dataSet));
        Q_EMIT isLoadingChanged();
        Q_EMIT done();
        return;
    }

    const auto &url = d->m_pendingChangeSets.front();
    if (url.isLocalFile()) {
        QFile f(url.toLocalFile());
        if (!f.open(QFile::ReadOnly)) {
            qCWarning(Log) << f.fileName() << f.errorString();
            d->m_errorMessage = f.errorString();
        } else {
            applyChangeSet(url, &f);
        }
    } else if (url.scheme() == "https"_L1) {
        QNetworkRequest req(url);
        req.setHeader(QNetworkRequest::UserAgentHeader, KOSMIndoorMap::userAgent());
        auto reply = d->m_nam()->get(req);
        connect(reply, &QNetworkReply::finished, this, [this, reply, url]() {
            reply->deleteLater();

            if (reply->error() != QNetworkReply::NoError) {
                d->m_errorMessage = reply->errorString();
            } else {
                applyChangeSet(url, reply);
            }

            d->m_pendingChangeSets.pop_front();
            applyNextChangeSet();
        });
        return;
    }

    d->m_pendingChangeSets.pop_front();
    applyNextChangeSet();
}

void MapLoader::applyChangeSet(const QUrl &url, QIODevice *io)
{
    auto reader = OSM::IO::readerForFileName(url.fileName(), &d->m_dataSet);
    if (!reader) {
        qCWarning(Log) << "unable to find reader for" << url;
        return;
    }

    reader->read(io);
    if (reader->hasError()) {
        d->m_errorMessage = reader->errorString();
    }
}

#include "moc_maploader.cpp"
