/*
    SPDX-FileCopyrightText: 2024 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "networkaccessmanagerfactory.h"

#include <QCoreApplication>
#include <QNetworkAccessManager>
#include <QNetworkDiskCache>
#include <QStandardPaths>

using namespace Qt::StringLiterals;

QNetworkAccessManager* KOSMIndoorMap::defaultNetworkAccessManagerFactory()
{
    static QNetworkAccessManager *s_nam = nullptr;

    if (!s_nam) {
        s_nam = new QNetworkAccessManager(QCoreApplication::instance());
        s_nam->setRedirectPolicy(QNetworkRequest::NoLessSafeRedirectPolicy);

        s_nam->enableStrictTransportSecurityStore(true, QStandardPaths::writableLocation(QStandardPaths::GenericCacheLocation) + "/org.kde.osm/hsts/"_L1);
        s_nam->setStrictTransportSecurityEnabled(true);

        auto namDiskCache = new QNetworkDiskCache(s_nam);
        namDiskCache->setCacheDirectory(QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + QLatin1String("/nam/"));
        s_nam->setCache(namDiskCache);
    }

    return s_nam;
}
