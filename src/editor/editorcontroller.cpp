/*
    SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "editorcontroller.h"
#include "logging.h"

#include <QCoreApplication>
#include <QDesktopServices>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>
#include <QUrlQuery>

using namespace KOSM;

static void openElementInId(OSM::Element element)
{
    QUrl url;
    url.setScheme(QStringLiteral("https"));
    url.setHost(QStringLiteral("www.openstreetmap.org"));
    url.setPath(QStringLiteral("/edit"));

    QUrlQuery query;
    query.addQueryItem(QStringLiteral("editor"), QStringLiteral("id"));
    switch (element.type()) {
        case OSM::Type::Null:
            Q_UNREACHABLE();
        case OSM::Type::Node:
            query.addQueryItem(QStringLiteral("node"), QString::number(element.id()));
            break;
        case OSM::Type::Way:
            query.addQueryItem(QStringLiteral("way"), QString::number(element.id()));
            break;
        case OSM::Type::Relation:
            query.addQueryItem(QStringLiteral("relation"), QString::number(element.id()));
            break;
    }

    url.setQuery(query);
    qCDebug(EditorLog) << url;
    QDesktopServices::openUrl(url);
}

static QUrl makeJosmLoadAndZoomCommand(OSM::Element element)
{
    QUrl url;
    url.setPath(QStringLiteral("/load_and_zoom"));

    QUrlQuery query;
    // ensure bbox is not 0x0 for nodes
    query.addQueryItem(QStringLiteral("left"), QString::number(element.boundingBox().min.lonF() - 0.0001));
    query.addQueryItem(QStringLiteral("bottom"), QString::number(element.boundingBox().min.latF() - 0.0001));
    query.addQueryItem(QStringLiteral("right"), QString::number(element.boundingBox().max.lonF() + 0.0001));
    query.addQueryItem(QStringLiteral("top"), QString::number(element.boundingBox().max.latF() + 0.0001));

    switch (element.type()) {
        case OSM::Type::Null:
            Q_UNREACHABLE();
        case OSM::Type::Node:
            query.addQueryItem(QStringLiteral("select"), QLatin1String("node") + QString::number(element.id()));
            break;
        case OSM::Type::Way:
            query.addQueryItem(QStringLiteral("select"), QLatin1String("way") + QString::number(element.id()));
            break;
        case OSM::Type::Relation:
            query.addQueryItem(QStringLiteral("select"), QLatin1String("relation") + QString::number(element.id()));
            break;
    }

    url.setQuery(query);
    return url;
}

static void openElementInVespucci(OSM::Element element)
{
    auto url = makeJosmLoadAndZoomCommand(element);
    url.setScheme(QStringLiteral("josm"));
    qCDebug(EditorLog) << url;
    QDesktopServices::openUrl(url);
}

#ifndef Q_OS_ANDROID
static std::unique_ptr<QNetworkAccessManager> s_nam;

static void openElementWithJosm(OSM::Element element)
{
    auto url = makeJosmLoadAndZoomCommand(element);
    url.setScheme(QStringLiteral("http"));
    url.setHost(QStringLiteral("127.0.0.1"));
    url.setPort(8111);
    qCDebug(EditorLog) << url;

    if (!s_nam) {
        s_nam.reset(new QNetworkAccessManager);
    }
    auto reply = s_nam->get(QNetworkRequest(url));
    QObject::connect(reply, &QNetworkReply::finished, QCoreApplication::instance(), [reply]() {
        reply->deleteLater();
        qCDebug(EditorLog) << reply->errorString();
        qCDebug(EditorLog) << reply->readAll();
    });
}
#endif

void EditorController::editElement(OSM::Element element)
{
    if (element.type() == OSM::Type::Null) {
        return;
    }

    qCDebug(EditorLog) << element.url();
    // TODO check which editor is actually installed
#ifdef Q_OS_ANDROID
    openElementInVespucci(element);
#else
    openElementWithJosm(element);
    // openElementInId(element);
#endif
}
