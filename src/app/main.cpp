/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <kosmindoormap_version.h>

#include <KLocalizedContext>
#include <KLocalizedString>

#include <QNetworkAccessManager>
#include <QNetworkDiskCache>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQmlNetworkAccessManagerFactory>
#include <QStandardPaths>

#ifdef Q_OS_ANDROID
#include <QGuiApplication>
#include <QQuickStyle>
#else
#include <QApplication>
#endif

#include <QCommandLineParser>
#include <QIcon>
#include <QTimer>
#include <QtPlugin>

#if HAVE_OSM_PBF_SUPPORT
Q_IMPORT_PLUGIN(OSM_PbfIOPlugin)
#endif

class NetworkAccessManagerFactory : public QQmlNetworkAccessManagerFactory
{
public:
    QNetworkAccessManager* create(QObject *parent) override
    {
        auto nam = new QNetworkAccessManager(parent);
        nam->setRedirectPolicy(QNetworkRequest::NoLessSafeRedirectPolicy);

        nam->enableStrictTransportSecurityStore(true, QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + QLatin1String("/hsts/"));
        nam->setStrictTransportSecurityEnabled(true);

        auto namDiskCache = new QNetworkDiskCache(nam);
        namDiskCache->setCacheDirectory(QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + QLatin1String("/nam/"));
        nam->setCache(namDiskCache);

        return nam;
    }
};

#ifdef Q_OS_ANDROID
Q_DECL_EXPORT
#endif
int main(int argc, char **argv)
{
    QCoreApplication::setApplicationName(QStringLiteral("kosmindoormap"));
    QCoreApplication::setOrganizationName(QStringLiteral("KDE"));
    QCoreApplication::setOrganizationDomain(QStringLiteral("kde.org"));
    QCoreApplication::setApplicationVersion(QStringLiteral(KOSMINDOORMAP_VERSION_STRING));

#ifdef Q_OS_ANDROID
    QGuiApplication app(argc, argv);
    QQuickStyle::setStyle(QStringLiteral("org.kde.breeze"));
#else
    QApplication app(argc, argv); // for native file dialogs
#endif
    QGuiApplication::setApplicationDisplayName(i18n("KDE OSM Indoor Map"));
    QGuiApplication::setWindowIcon(QIcon::fromTheme(QStringLiteral("go-home")));

    QCommandLineParser parser;
    QCommandLineOption selfTestOpt(QStringLiteral("self-test"), QStringLiteral("internal, for automated testing"));
    parser.addOption(selfTestOpt);
    parser.process(app);

    QQmlApplicationEngine engine;
    engine.setNetworkAccessManagerFactory(new NetworkAccessManagerFactory());
    auto l10nContext = new KLocalizedContext(&engine);
    l10nContext->setTranslationDomain(QStringLiteral(TRANSLATION_DOMAIN));
    engine.rootContext()->setContextObject(l10nContext);

    engine.loadFromModule("org.kde.kosmindoormap.app", "Main");

    if (parser.isSet(selfTestOpt)) {
        QTimer::singleShot(std::chrono::milliseconds(250), &app, &QCoreApplication::quit);
    }

    return app.exec();
}
