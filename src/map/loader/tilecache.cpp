/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "tilecache_p.h"
#include "logging.h"

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

TileCache::TileCache(QObject *parent)
    : QObject(parent)
    , m_nam(new QNetworkAccessManager(this))
{
    m_nam->setRedirectPolicy(QNetworkRequest::NoLessSafeRedirectPolicy);
    m_nam->enableStrictTransportSecurityStore(true, QStandardPaths::writableLocation(QStandardPaths::GenericCacheLocation) + QLatin1String("/org.kde.osm/hsts/"));
    m_nam->setStrictTransportSecurityEnabled(true);
}

TileCache::~TileCache() = default;

QString TileCache::cachedTile(Tile tile) const
{
    const auto p = cachePath(tile);
    if (QFile::exists(p)) {
        return p;
    }
    return {};
}

void TileCache::ensureCached(Tile tile)
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

void TileCache::downloadTile(Tile tile)
{
    m_pendingDownloads.push_back(tile);
    downloadNext();
}

QString TileCache::cachePath(Tile tile) const
{
    QString base;
    if (!qEnvironmentVariableIsSet("KOSMINDOORMAP_CACHE_PATH")) {
        base = QStandardPaths::writableLocation(QStandardPaths::GenericCacheLocation)
            + QLatin1String("/org.kde.osm/vectorosm/");
    } else {
        base = qEnvironmentVariable("KOSMINDOORMAP_CACHE_PATH");
    }

    return base
        + QString::number(tile.z) + QLatin1Char('/')
        + QString::number(tile.x) + QLatin1Char('/')
        + QString::number(tile.y) + QLatin1String(".o5m");
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
    m_output.setFileName(fi.absoluteFilePath() + QLatin1String(".part"));
    if (!m_output.open(QFile::WriteOnly)) {
        qCWarning(Log) << m_output.fileName() << m_output.errorString();
        return;
    }

    QUrl url;
    if (qEnvironmentVariableIsSet("KOSMINDOORMAP_TILESERVER")) {
        url = QUrl(qEnvironmentVariable("KOSMINDOORMAP_TILESERVER"));
    } else {
        url.setScheme(QStringLiteral("https"));
        url.setHost(QStringLiteral("maps.kde.org"));
        url.setPath(QStringLiteral("/earth/vectorosm/v1/"));
    }

    url.setPath(url.path() + QString::number(tile.z) + QLatin1Char('/')
        + QString::number(tile.x) + QLatin1Char('/')
        + QString::number(tile.y) + QLatin1String(".o5m"));

    QNetworkRequest req(url);
    req.setAttribute(QNetworkRequest::Http2AllowedAttribute, true);
    req.setHeader(QNetworkRequest::UserAgentHeader, (QCoreApplication::applicationName() + QLatin1Char('/') + QCoreApplication::applicationVersion()).toUtf8());
    auto reply = m_nam->get(req);
    connect(reply, &QNetworkReply::readyRead, this, [this, reply]() { dataReceived(reply); });
    connect(reply, &QNetworkReply::finished, this, [this, reply, tile]() { downloadFinished(reply, tile); });
    connect(reply, &QNetworkReply::sslErrors, this, [reply](const auto &sslErrors) { reply->setProperty("_ssl_errors", QVariant::fromValue(sslErrors)); });
}

void TileCache::dataReceived(QNetworkReply *reply)
{
    m_output.write(reply->read(reply->bytesAvailable()));
}

void TileCache::downloadFinished(QNetworkReply* reply, Tile tile)
{
    reply->deleteLater();
    m_output.close();

    if (reply->error() != QNetworkReply::NoError) {
        qCWarning(Log) << reply->errorString() << reply->url();
        m_output.remove();
        if (reply->error() == QNetworkReply::SslHandshakeFailedError) {
            const auto sslErrors = reply->property("_ssl_errors").value<QList<QSslError>>();
            QStringList errorStrings;
            errorStrings.reserve(sslErrors.size());
            std::transform(sslErrors.begin(), sslErrors.end(), std::back_inserter(errorStrings), [](const auto &e) { return e.errorString(); });
            qCWarning(Log) << errorStrings;
            Q_EMIT tileError(tile, reply->errorString() + QLatin1String(" (") + errorStrings.join(QLatin1String(", ")) + QLatin1Char(')'));
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
    return m_pendingDownloads.size() + (m_output.isOpen() ? 1 : 0);
}

void TileCache::cancelPending()
{
    m_pendingDownloads.clear();
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
    const QString base = QStandardPaths::writableLocation(QStandardPaths::GenericCacheLocation) + QLatin1String("/org.kde.osm/vectorosm/");
    expireRecursive(base);
}

void TileCache::updateTtl(const QString &filePath, const QDateTime &ttl)
{
    QFile f(filePath);
    f.open(QFile::WriteOnly | QFile::Append);
    f.setFileTime(std::max(f.fileTime(QFileDevice::FileModificationTime), ttl), QFile::FileModificationTime);
}

#include "moc_tilecache_p.cpp"
