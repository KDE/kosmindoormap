/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "tilecache_p.h"
#include "logging.h"
#include "network/useragent_p.h"

#include <osm/datatypes.h>
#include <osm/geomath.h>

#include <QCoreApplication>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QStandardPaths>
#include <QUrl>

#include <cmath>

using namespace Qt::Literals;
using namespace KOSMIndoorMap;

enum {
    DefaultCacheDays = 14,
};

Tile Tile::fromCoordinate(double lat, double lon, uint8_t z)
{
    Tile t;
    t.x = std::floor((lon + 180.0) / 360.0 * (1 << z));
    const auto latrad = OSM::degToRad(lat);
    t.y = std::floor((1.0 - std::asinh(std::tan(latrad)) / M_PI) / 2.0 * (1 << z));
    t.z = z;
    return t;
}

OSM::Coordinate Tile::topLeft() const
{
    const auto lon = x / (double)(1 << z) * 360.0 - 180.0;

    const auto n = M_PI - 2.0 * M_PI * y / (double)(1 << z);
    const auto lat = OSM::radToDeg(std::atan(0.5 * (std::exp(n) - std::exp(-n))));

    return OSM::Coordinate(lat, lon);
}

OSM::BoundingBox Tile::boundingBox() const
{
    Tile bottomRight = *this;
    ++bottomRight.x;
    ++bottomRight.y;

    const auto tl = topLeft();
    const auto br = bottomRight.topLeft();

    return OSM::BoundingBox(OSM::Coordinate(br.latitude, tl.longitude), OSM::Coordinate(tl.latitude, br.longitude));
}

Tile Tile::topLeftAtZ(uint8_t z) const
{
    if (z == this->z) {
        return *this;
    }
    if (z < this->z) {
        return Tile{ x / (1 << (this->z - z)), y / (1 << (this->z - z)), z};
    }
    return Tile{ x * (1 << (z - this->z )), y * (1 << (z - this->z)), z};
}

Tile Tile::bottomRightAtZ(uint8_t z) const
{
    if (z <= this->z) {
        return topLeftAtZ(z);
    }
    const auto deltaZ = z - this->z;
    const auto deltaWidth = 1 << deltaZ;
    return Tile{ x * deltaWidth + deltaWidth - 1, y * deltaWidth + deltaWidth - 1, z};
}

TileCache::TileCache(const NetworkAccessManagerFactory &namFactory, QObject *parent)
    : QObject(parent)
    , m_nam(namFactory)
{
}

TileCache::~TileCache() = default;

QString TileCache::cachedTile(const Tile &tile) const
{
    auto p = cachePath(tile);
    if (QFileInfo info(p); info.exists() && p.size() > 0) {
        return p;
    }
    return {};
}

void TileCache::ensureCached(const Tile &tile)
{
    const auto t = cachedTile(tile);
    if (t.isEmpty()) {
        downloadTile(tile);
        return;
    }

    if (tile.ttl.isValid()) {
        updateTtl(t, tile.ttl);
    }
}

void TileCache::downloadTile(const Tile &tile)
{
    m_pendingDownloads.push_back(tile);
    downloadNext();
}

QString TileCache::cachePath(const Tile &tile) const
{
    QString base;
    if (!qEnvironmentVariableIsSet("KOSMINDOORMAP_CACHE_PATH")) {
        base = QStandardPaths::writableLocation(QStandardPaths::GenericCacheLocation)
            + "/org.kde.osm/vectorosm/"_L1;
    } else {
        base = qEnvironmentVariable("KOSMINDOORMAP_CACHE_PATH");
    }

    return base
        + QString::number(tile.z) + '/'_L1
        + QString::number(tile.x) + '/'_L1
        + QString::number(tile.y) + ".o5m"_L1;
}

void TileCache::downloadNext()
{
    if (m_output.isOpen() || m_pendingDownloads.empty()) {
        return;
    }

    const auto tile = m_pendingDownloads.front();
    m_pendingDownloads.pop_front();

    QFileInfo fi(cachePath(tile));
    QDir().mkpath(fi.absolutePath());
    m_output.setFileName(fi.absoluteFilePath() + ".part"_L1);
    if (!m_output.open(QFile::WriteOnly)) {
        qCWarning(Log) << m_output.fileName() << m_output.errorString();
        return;
    }

    QUrl url;
    if (qEnvironmentVariableIsSet("KOSMINDOORMAP_TILESERVER")) {
        url = QUrl(qEnvironmentVariable("KOSMINDOORMAP_TILESERVER"));
    } else {
        url.setScheme(u"https"_s);
        url.setHost(u"maps.kde.org"_s);
        url.setPath(u"/earth/vectorosm/v1/"_s);
    }

    url.setPath(url.path() + QString::number(tile.z) + '/'_L1
        + QString::number(tile.x) + '/'_L1
        + QString::number(tile.y) + ".o5m"_L1);

    QNetworkRequest req(url);
    req.setAttribute(QNetworkRequest::Http2AllowedAttribute, true);
    req.setAttribute(QNetworkRequest::HttpPipeliningAllowedAttribute, true);
    // bypass cache, we manage that ourselves
    req.setAttribute(QNetworkRequest::CacheLoadControlAttribute,  QNetworkRequest::AlwaysNetwork);
    req.setAttribute(QNetworkRequest::CacheSaveControlAttribute,  false);
    req.setHeader(QNetworkRequest::UserAgentHeader, KOSMIndoorMap::userAgent());
    auto reply = m_nam()->get(req);
    reply->setParent(this);
    connect(reply, &QNetworkReply::readyRead, this, [this, reply]() { dataReceived(reply); });
    connect(reply, &QNetworkReply::finished, this, [this, reply, tile]() { downloadFinished(reply, tile); });
    connect(reply, &QNetworkReply::sslErrors, this, [reply](const auto &sslErrors) { reply->setProperty("_ssl_errors", QVariant::fromValue(sslErrors)); });
    m_currentReply = reply;
}

void TileCache::dataReceived(QNetworkReply *reply)
{
    m_output.write(reply->read(reply->bytesAvailable()));
}

void TileCache::downloadFinished(QNetworkReply* reply, const Tile &tile)
{
    reply->deleteLater();
    m_currentReply = {};
    m_output.close();

    if (reply->error() != QNetworkReply::NoError || m_output.size() == 0) {
        qCWarning(Log) << reply->errorString() << reply->url();
        m_output.remove();
        if (reply->error() == QNetworkReply::SslHandshakeFailedError) {
            const auto sslErrors = reply->property("_ssl_errors").value<QList<QSslError>>();
            QStringList errorStrings;
            errorStrings.reserve(sslErrors.size());
            std::transform(sslErrors.begin(), sslErrors.end(), std::back_inserter(errorStrings), [](const auto &e) { return e.errorString(); });
            qCWarning(Log) << errorStrings;
            Q_EMIT tileError(tile, reply->errorString() + " ("_L1 + errorStrings.join(", "_L1) + ')'_L1);
        } else {
            Q_EMIT tileError(tile, reply->errorString());
        }
        downloadNext();
        return;
    }

    const auto t = cachePath(tile);
    m_output.rename(t);
    if (tile.ttl.isValid()) {
        updateTtl(t, std::max(QDateTime::currentDateTimeUtc().addDays(1), tile.ttl));
    } else {
        updateTtl(t, QDateTime::currentDateTimeUtc().addDays(DefaultCacheDays));
    }

    Q_EMIT tileLoaded(tile);
    downloadNext();
}

int TileCache::pendingDownloads() const
{
    return (int)m_pendingDownloads.size() + (m_output.isOpen() ? 1 : 0);
}

void TileCache::cancelPending()
{
    m_pendingDownloads.clear();
    delete m_currentReply.get();
    if (m_output.isOpen()) {
        m_output.close();
        m_output.remove();
    }
}

static void expireRecursive(const QString &path)
{
    QDirIterator it(path, QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot | QDir::NoSymLinks);
    while (it.hasNext()) {
        it.next();

        if (it.fileInfo().isDir()) {
            expireRecursive(it.filePath());
            if (QDir(it.filePath()).isEmpty()) {
                qCDebug(Log) << "removing empty tile directory" << it.fileName();
                QDir(path).rmdir(it.filePath());
            }
        } else if (it.fileInfo().lastModified() < QDateTime::currentDateTimeUtc()) {
            qCDebug(Log) << "removing expired tile" << it.filePath();
            QDir(path).remove(it.filePath());
        }
    }
}
void TileCache::expire()
{
    const QString base = QStandardPaths::writableLocation(QStandardPaths::GenericCacheLocation) + "/org.kde.osm/vectorosm/"_L1;
    expireRecursive(base);
}

void TileCache::updateTtl(const QString &filePath, const QDateTime &ttl)
{
    QFile f(filePath);
    f.open(QFile::WriteOnly | QFile::Append);
    f.setFileTime(std::max(f.fileTime(QFileDevice::FileModificationTime), ttl), QFile::FileModificationTime);
}

#include "moc_tilecache_p.cpp"
