/*
    SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <config-editorcontroller.h>

#include "editorcontroller.h"
#include "logging.h"

#if HAVE_KSERVICE
#include <KService>
#include <KShell>
#endif

#include <QCoreApplication>
#include <QDesktopServices>
#include <QElapsedTimer>
#include <QHostAddress>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QProcess>
#include <QTcpSocket>
#include <QTimer>
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

#ifdef Q_OS_ANDROID
static void openElementInVespucci(OSM::Element element)
{
    auto url = makeJosmLoadAndZoomCommand(element);
    url.setScheme(QStringLiteral("josm"));
    qCDebug(EditorLog) << url;
    QDesktopServices::openUrl(url);
}

#else

static std::unique_ptr<QNetworkAccessManager> s_nam;

static void josmRemoteCommand(const QUrl &url, QElapsedTimer timeout)
{
   if (!s_nam) {
        s_nam = std::make_unique<QNetworkAccessManager>();
    }
    auto reply = s_nam->get(QNetworkRequest(url));
    QObject::connect(reply, &QNetworkReply::finished, QCoreApplication::instance(), [reply, url, timeout]() {
        reply->deleteLater();
        qCDebug(EditorLog) << reply->errorString();
        qCDebug(EditorLog) << reply->readAll();
        // retry in case JOSM is still starting up
        if (reply->error() != QNetworkReply::NoError && timeout.elapsed() < 30000) {
            QTimer::singleShot(1000, QCoreApplication::instance(), [url, timeout]() { josmRemoteCommand(url, timeout); });
        }
    });
}

static void openElementWithJosm(OSM::Element element)
{
    // TODO start josm if not yet running
#if HAVE_KSERVICE
    QTcpSocket socket;
    socket.connectToHost(QHostAddress::LocalHost, 8111);
    if (!socket.waitForConnected(100)) {
        qCDebug(EditorLog) << "JOSM not running yet, or doesn't have remote control enabled." << socket.errorString();
        auto s = KService::serviceByDesktopName(QStringLiteral("org.openstreetmap.josm"));
        qCDebug(EditorLog) << "JOSM not running yet, or doesn't have remote control enabled." << s->exec();
        Q_ASSERT(s);
        auto args = KShell::splitArgs(s->exec());
        if (args.isEmpty()) {
            return;
        }
        const auto program = args.takeFirst();
        QProcess::startDetached(program, args);
    }
    socket.close();
#endif

    auto url = makeJosmLoadAndZoomCommand(element);
    url.setScheme(QStringLiteral("http"));
    url.setHost(QStringLiteral("127.0.0.1"));
    url.setPort(8111);
    qCDebug(EditorLog) << url;

    QElapsedTimer timeout;
    timeout.start();
    josmRemoteCommand(url, timeout);
}
#endif

bool EditorController::hasEditor(Editor editor)
{
    switch (editor) {
        case ID:
            return true;
        case JOSM:
#if HAVE_KSERVICE
        {
            auto s = KService::serviceByDesktopName(QStringLiteral("org.openstreetmap.josm"));
            return s;
        }
#else
            return false;
#endif
        case Vespucci:
#ifdef Q_OS_ANDROID
            // TODO
            return true;
#else
            return false;
#endif
    }

    return false;
}

void EditorController::editElement(OSM::Element element)
{
    if (element.type() == OSM::Type::Null) {
        return;
    }

    qCDebug(EditorLog) << element.url();
#ifdef Q_OS_ANDROID
    if (hasEditor(Vespucci)) {
        openElementInVespucci(element);
    } else {
        openElementInVespucci(element);
    }
#else
    if (hasEditor(JOSM)) {
        openElementWithJosm(element);
    } else {
        openElementInId(element);
    }
#endif
}
