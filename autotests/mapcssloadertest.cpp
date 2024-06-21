/*
    SPDX-FileCopyrightText: 2024 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <map/style/mapcssloader.h>
#include <map/style/mapcssstyle.h>

#include <QSignalSpy>
#include <QStandardPaths>
#include <QTest>

using namespace Qt::Literals::StringLiterals;
using namespace KOSMIndoorMap;

class MapCSSLoaderTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase()
    {
        QStandardPaths::setTestModeEnabled(true);
    }

    void testResolve_data()
    {
        QTest::addColumn<QString>("style");
        QTest::addColumn<QString>("baseUrl");
        QTest::addColumn<QString>("resolved");

        QTest::newRow("empty") << QString() << QString() << u"qrc:///org.kde.kosmindoormap/assets/css/breeze-light.mapcss"_s;
        QTest::newRow("default") << u"default"_s << QString() << u"qrc:///org.kde.kosmindoormap/assets/css/breeze-light.mapcss"_s;
        QTest::newRow("breeze-light") << u"breeze-light"_s << QString() << u"qrc:///org.kde.kosmindoormap/assets/css/breeze-light.mapcss"_s;
        QTest::newRow("breeze-dark") << u"breeze-dark"_s << QString() << u"qrc:///org.kde.kosmindoormap/assets/css/breeze-dark.mapcss"_s;
        QTest::newRow("relative-unknown") << u"i-dont-exist"_s << QString() << QString();
        QTest::newRow("relative-default") << u"breeze-dark.mapcss"_s << QString() << u"qrc:///org.kde.kosmindoormap/assets/css/breeze-dark.mapcss"_s;
        QTest::newRow("relative-qrc-base") << u"breeze-base.mapcss"_s << u"qrc:///org.kde.kosmindoormap/assets/css/breeze-dark.mapcss"_s << u"qrc:///org.kde.kosmindoormap/assets/css/breeze-base.mapcss"_s;
        QTest::newRow("relative-http-base") << u"breeze-base.mapcss"_s << u"https://invent.kde.org/vkrause/kosmindoormap/-/raw/work/vkrause/akademy-2024/src/map/assets/css/akademy2024-dark.mapcss"_s << u"https://invent.kde.org/vkrause/kosmindoormap/-/raw/work/vkrause/akademy-2024/src/map/assets/css/breeze-base.mapcss"_s;
        QTest::newRow("relative-to-local-http-base") << u"breeze-common.mapcss"_s << u"https://invent.kde.org/vkrause/kosmindoormap/-/raw/work/vkrause/akademy-2024/src/map/assets/css/akademy2024-dark.mapcss"_s << u"qrc:///org.kde.kosmindoormap/assets/css/breeze-common.mapcss"_s;
        // TODO absolute file with/without context
        QTest::newRow("absolute-http-no-base") << u"https://invent.kde.org/vkrause/kosmindoormap/-/raw/work/vkrause/akademy-2024/src/map/assets/css/akademy2024-dark.mapcss"_s << QString() << u"https://invent.kde.org/vkrause/kosmindoormap/-/raw/work/vkrause/akademy-2024/src/map/assets/css/akademy2024-dark.mapcss"_s;
        QTest::newRow("absolute-http-base") << u"https://invent.kde.org/vkrause/kosmindoormap/-/raw/work/vkrause/akademy-2024/src/map/assets/css/akademy2024-dark.mapcss"_s << u"https://kde.org"_s << u"https://invent.kde.org/vkrause/kosmindoormap/-/raw/work/vkrause/akademy-2024/src/map/assets/css/akademy2024-dark.mapcss"_s;
        QTest::newRow("reject-http") << u"http://invent.kde.org/vkrause/kosmindoormap/-/raw/work/vkrause/akademy-2024/src/map/assets/css/akademy2024-dark.mapcss"_s << QString() << QString();
    }

    void testResolve()
    {
        QFETCH(QString, style);
        QFETCH(QString, baseUrl);
        QFETCH(QString, resolved);

        QCOMPARE(MapCSSLoader::resolve(style, QUrl(baseUrl)), QUrl(resolved));
    }

    void testToLocalFile_data()
    {
        QTest::addColumn<QString>("url");
        QTest::addColumn<QString>("path");

        QTest::newRow("empty") << QString() << QString();
        QTest::newRow("file") << u"file:///usr/share/org.kde.kosmindoormap/assets/css/breeze-light.mapcss"_s << u"/usr/share/org.kde.kosmindoormap/assets/css/breeze-light.mapcss"_s;
        QTest::newRow("qrc") << u"qrc:///org.kde.kosmindoormap/assets/css/breeze-light.mapcss"_s << u":/org.kde.kosmindoormap/assets/css/breeze-light.mapcss"_s;
        QTest::newRow("http") << u"https://invent.kde.org/vkrause/kosmindoormap/-/raw/work/vkrause/akademy-2024/src/map/assets/css/akademy2024-dark.mapcss"_s << u"/cache/org.kde.osm/mapcss/3ea95b43e74ed0f713a76782e479d0decdee4730"_s;
    }

    void testToLocalFile()
    {
        QFETCH(QString, url);
        QFETCH(QString, path);

        qDebug() << MapCSSLoader::toLocalFile(QUrl(url));
        QVERIFY(MapCSSLoader::toLocalFile(QUrl(url)).endsWith(path));
    }

    void testDefaulLoad()
    {
        MapCSSLoader loader(MapCSSLoader::resolve("default"_L1), nullptr);
        QSignalSpy finishedSpy(&loader, &MapCSSLoader::finished);
        loader.start();
        QCOMPARE(finishedSpy.size(), 1);
        QCOMPARE(loader.hasError(), false);
        QVERIFY(loader.errorMessage().isEmpty());
        auto style = loader.takeStyle();
        QVERIFY(!style.isEmpty());
    }

    void testExpire()
    {
        MapCSSLoader::expire();
    }

    void testSyntaxError()
    {
        MapCSSLoader loader(MapCSSLoader::resolve(QStringLiteral(SOURCE_DIR "/data/mapcss/parser-test.mapcss.ref.license")), nullptr);
        QSignalSpy finishedSpy(&loader, &MapCSSLoader::finished);
        loader.start();
        QCOMPARE(finishedSpy.size(), 1);
        QCOMPARE(loader.hasError(), true);
        QCOMPARE(loader.errorMessage().isEmpty(), false);
    }
};

QTEST_GUILESS_MAIN(MapCSSLoaderTest)

#include "mapcssloadertest.moc"
