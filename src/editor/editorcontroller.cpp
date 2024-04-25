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

#ifdef Q_OS_ANDROID
#include <QJniObject>
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

using namespace Qt::Literals::StringLiterals;
using namespace KOSM;

// https://github.com/openstreetmap/iD/blob/develop/API.md
static void openElementInId(OSM::Element element)
{
    QUrl url;
    url.setScheme(QStringLiteral("https"));
    url.setHost(QStringLiteral("www.openstreetmap.org"));
    url.setPath(QStringLiteral("/edit"));

    QUrlQuery query;
    query.addQueryItem(QStringLiteral("editor"), QStringLiteral("id"));
    query.addQueryItem(QLatin1StringView(OSM::typeName(element.type())), QString::number(element.id()));

    url.setQuery(query);
    qCDebug(EditorLog) << url;
    QDesktopServices::openUrl(url);
}

static void openBoundBoxInId(OSM::BoundingBox box)
{
    QUrl url;
    url.setScheme(QStringLiteral("https"));
    url.setHost(QStringLiteral("www.openstreetmap.org"));
    url.setPath(QStringLiteral("/edit"));

    QUrlQuery query;
    query.addQueryItem(QStringLiteral("editor"), QStringLiteral("id"));
    query.addQueryItem(QStringLiteral("lat"), QString::number(box.center().latF()));
    query.addQueryItem(QStringLiteral("lon"), QString::number(box.center().lonF()));
    query.addQueryItem(QStringLiteral("zoom"), QStringLiteral("17")); // TODO compute zoom based on box size

    url.setQuery(query);
    QDesktopServices::openUrl(url);
}

static QUrl makeJosmLoadAndZoomCommand(OSM::BoundingBox box, OSM::Element element)
{
    QUrl url;
    url.setPath(QStringLiteral("/load_and_zoom"));

    QUrlQuery query;
    // ensure bbox is not 0x0 for nodes
    query.addQueryItem(QStringLiteral("left"), QString::number(box.min.lonF() - 0.0001));
    query.addQueryItem(QStringLiteral("bottom"), QString::number(box.min.latF() - 0.0001));
    query.addQueryItem(QStringLiteral("right"), QString::number(box.max.lonF() + 0.0001));
    query.addQueryItem(QStringLiteral("top"), QString::number(box.max.latF() + 0.0001));
    query.addQueryItem(QStringLiteral("select"), QLatin1StringView(OSM::typeName(element.type())) + QString::number(element.id()));

    url.setQuery(query);
    return url;
}

#ifdef Q_OS_ANDROID
// https://vespucci.io/tutorials/vespucci_intents/
static void openVespucci(OSM::BoundingBox box, OSM::Element element = {})
{
    auto url = makeJosmLoadAndZoomCommand(box, element);
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

// https://josm.openstreetmap.de/wiki/Help/RemoteControlCommands
static void openJosm(OSM::BoundingBox box, OSM::Element element = {})
{
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

    auto url = makeJosmLoadAndZoomCommand(box, element);
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
            return QJniObject::callStaticMethod<jboolean>("org.kde.osm.editorcontroller.EditorController",
                "hasVespucci", "(Landroid/content/Context;)Z", QNativeInterface::QAndroidApplication::context());
#else
            return false;
#endif
    }

    return false;
}

void EditorController::editElement(OSM::Element element, Editor editor)
{
    if (element.type() == OSM::Type::Null) {
        return;
    }

    qCDebug(EditorLog) << element.url() << editor;
    switch (editor) {
        case ID:
            openElementInId(element);
            break;
        case JOSM:
#ifndef Q_OS_ANDROID
            openJosm(element.boundingBox(), element);
#endif
            break;
        case Vespucci:
#ifdef Q_OS_ANDROID
            openVespucci(element.boundingBox(), element);
#endif
            break;
    }
}

void EditorController::editBoundingBox(OSM::BoundingBox box, Editor editor)
{
    qCDebug(EditorLog) << box << editor;
    switch (editor) {
        case ID:
            openBoundBoxInId(box);
            break;
        case JOSM:
#ifndef Q_OS_ANDROID
            openJosm(box);
#endif
            break;
        case Vespucci:
#ifdef Q_OS_ANDROID
            openVespucci(box);
#endif
            break;
    }
}

#include "moc_editorcontroller.cpp"
