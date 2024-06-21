/*
    SPDX-FileCopyrightText: 2024 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "mapcssloader.h"
#include "mapcssparser.h"
#include "mapcssstyle.h"
#include "logging.h"
#include "network/useragent_p.h"

#include <QCryptographicHash>
#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QGuiApplication>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QPalette>
#include <QStandardPaths>

using namespace Qt::Literals::StringLiterals;
using namespace KOSMIndoorMap;

class KOSMIndoorMap::MapCSSLoaderPrivate
{
public:
    QUrl m_styleUrl;
    MapCSSStyle m_style;
    MapCSSParser::Error m_error = MapCSSParser::SyntaxError;
    QString m_errorMsg;
    QSet<QUrl> m_alreadyDownloaded;
    NetworkAccessManagerFactory m_nam;
};

MapCSSLoader::MapCSSLoader(const QUrl &style, const NetworkAccessManagerFactory &nam, QObject *parent)
    : QObject(parent)
    , d(std::make_unique<MapCSSLoaderPrivate>())
{
    d->m_styleUrl = style;
    d->m_nam = nam;
}

MapCSSLoader::~MapCSSLoader() = default;

void MapCSSLoader::start()
{
    MapCSSParser p;
    d->m_style = p.parse(d->m_styleUrl);
    d->m_error = p.error();
    d->m_errorMsg = p.errorMessage();

    if (d->m_error == MapCSSParser::FileNotFoundError) {
        download(p.url());
    } else {
        Q_EMIT finished();
    }
}

MapCSSStyle&& MapCSSLoader::takeStyle()
{
    return std::move(d->m_style);
}

bool MapCSSLoader::hasError() const
{
    return d->m_error != MapCSSParser::NoError;
}

QString MapCSSLoader::errorMessage() const
{
    return d->m_errorMsg;
}

[[nodiscard]] static QUrl pathToUrl(const QString &path)
{
    if (path.startsWith(':'_L1)) {
        QUrl url;
        url.setScheme(u"qrc"_s);
        url.setHost(u""_s);
        url.setPath(path.mid(1));
        return url;
    }

    return QUrl::fromLocalFile(path);
}

QUrl MapCSSLoader::resolve(const QString &style, const QUrl &baseUrl)
{
    if (style.isEmpty() || style == "default"_L1) {
        if (qobject_cast<QGuiApplication*>(QGuiApplication::instance()) && QGuiApplication::palette().base().color().value() < 128) {
            return resolve(u"breeze-dark"_s, baseUrl);
        }
        return resolve(u"breeze-light"_s, baseUrl);
    }

    if (style.startsWith("http://"_L1)) {
        qCWarning(Log) << "not loading MapCSS from insecure HTTP source!" << style;
        return {};
    }

    if (style.startsWith("https://"_L1)) {
        return QUrl(style);
    }

    QString fileName = style;
    if (style.startsWith("file:/"_L1) || style.startsWith("qrc:/"_L1)) {
        fileName = toLocalFile(QUrl(style));
    }

    QFileInfo fi(fileName);
    if (fi.isAbsolute()) {
        return pathToUrl(fi.absoluteFilePath());
    }

    QUrl resolved;
    if (!baseUrl.isEmpty()) {
        resolved = baseUrl.resolved(QUrl(style));
        if (QFile::exists(toLocalFile(resolved))) {
            return resolved;
        }
    }

#ifndef Q_OS_ANDROID
    auto searchPaths = QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation);
#else
    auto searchPaths = QStandardPaths::standardLocations(QStandardPaths::AppLocalDataLocation);
#endif
    searchPaths.push_back(u":"_s);
    for (const auto &searchPath : std::as_const(searchPaths)) {
        QString f = searchPath + "/org.kde.kosmindoormap/assets/css/"_L1 + style + (style.endsWith(".mapcss"_L1) ? ""_L1 : ".mapcss"_L1);
        if (QFile::exists(f)) {
            qCDebug(Log) << "resolved stylesheet" << style << "to" << f;
            return pathToUrl(f);
        }
    }

    return resolved;
}

[[nodiscard]] static QString cacheBasePath()
{
    return QStandardPaths::writableLocation(QStandardPaths::GenericCacheLocation) + "/org.kde.osm/mapcss/"_L1;
}

QString MapCSSLoader::toLocalFile(const QUrl &url)
{
    if (url.isLocalFile() || url.scheme() == "file"_L1) {
        return url.toLocalFile();
    }
    if (url.scheme() == "qrc"_L1) {
        return ':'_L1 + url.path();
    }

    if (url.scheme() == "https"_L1) {
        return cacheBasePath() + QString::fromLatin1(QCryptographicHash::hash(url.toString().toUtf8(), QCryptographicHash::Sha1).toHex());
    }

    return {};
}

void MapCSSLoader::expire()
{
    const auto expireDt = QDateTime::currentDateTimeUtc().addDays(-1);
    for (QDirIterator it(cacheBasePath(), QDir::Files | QDir::NoSymLinks); it.hasNext();) {
        it.next();
        if (it.fileInfo().lastModified() < expireDt) {
            qCDebug(Log) << "expiring" << it.filePath();
            QFile::remove(it.filePath());
        }
    }
}

void MapCSSLoader::download(const QUrl &url)
{
    // don't try to download the same thing twice, even if we fail due to network issues etc
    if (!url.isValid() || url.scheme() != "https"_L1 || d->m_alreadyDownloaded.contains(url)) {
        Q_EMIT finished();
        return;
    }
    d->m_alreadyDownloaded.insert(url);

    QNetworkRequest req(url);
    req.setAttribute(QNetworkRequest::Http2AllowedAttribute, true);
    req.setAttribute(QNetworkRequest::HttpPipeliningAllowedAttribute, true);
    req.setHeader(QNetworkRequest::UserAgentHeader, KOSMIndoorMap::userAgent());
    qCDebug(Log) << "retrieving" << url;
    auto reply = d->m_nam()->get(req);
    reply->setParent(this);
    connect(reply, &QNetworkReply::finished, this, [this, reply, url]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            d->m_errorMsg = reply->errorString();
            d->m_error = MapCSSParser::NetworkError;
            Q_EMIT finished();
            return;
        }

        QDir().mkpath(cacheBasePath());
        QFile cacheFile(MapCSSLoader::toLocalFile(url));
        if (!cacheFile.open(QFile::WriteOnly)) {
            d->m_errorMsg = cacheFile.errorString();
            d->m_error = MapCSSParser::FileIOError;
            Q_EMIT finished();
            return;
        }
        cacheFile.write(reply->readAll());
        cacheFile.close();

        start();
    });
}

#include "moc_mapcssloader.cpp"
