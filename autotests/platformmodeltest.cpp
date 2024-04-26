/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <map/loader/mapdata.h>
#include <map/loader/maploader.h>
#include <map/content/platformmodel.h>

#include <QTest>
#include <QAbstractItemModelTester>
#include <QSignalSpy>

using namespace KOSMIndoorMap;

class PlatformModelTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testPlatformModel()
    {
        MapLoader loader;
        loader.loadFromFile(QStringLiteral(SOURCE_DIR "/data/platforms/hamburg-altona.osm"));
        QCOMPARE(loader.isLoading(), false);
        QCOMPARE(loader.hasError(), false);

        auto mapData = loader.takeData();
        QVERIFY(!mapData.dataSet().nodes.empty());
        QVERIFY(!mapData.dataSet().ways.empty());
        QVERIFY(!mapData.dataSet().relations.empty());
        QVERIFY(mapData.boundingBox().height() > 0);
        QVERIFY(mapData.boundingBox().width() > 0);

        PlatformModel model;
        QAbstractItemModelTester modelTest(&model);
        QSignalSpy platformChangeSpy(&model, &PlatformModel::platformIndexChanged);

        model.setMapData(mapData);
        Platform p;
        p.setMode(Platform::Rail);
        p.setName(QStringLiteral("10"));
        model.setArrivalPlatform(p);
        p.setName(QStringLiteral("7"));
        model.setDeparturePlatform(p);
        QVERIFY(platformChangeSpy.wait());
        QCOMPARE(model.rowCount(), 12);

        for (int i = 0; i < model.rowCount(); ++i) {
            const auto idx = model.index(i, 0);
            QVERIFY(!idx.data(Qt::DisplayRole).toString().isEmpty());
            QVERIFY(idx.data(PlatformModel::ElementRole).value<OSM::Element>().type() != OSM::Type::Null);

            const auto secCount = model.rowCount(idx);
            QVERIFY(secCount > 0 || !(idx.data(PlatformModel::DeparturePlatformRole).toBool() || idx.data(PlatformModel::ArrivalPlatformRole).toBool()));
            for (int j = 0; j < secCount; ++j) {
                const auto secIdx = model.index(j, 0, idx);
                QVERIFY(!secIdx.data(Qt::DisplayRole).toString().isEmpty());
                const auto secElem = secIdx.data(PlatformModel::ElementRole).value<OSM::Element>();
                QVERIFY(secElem.type() != OSM::Type::Null);
                QVERIFY(secElem.tagValue("mx:arrival").isEmpty() || secElem.tagValue("mx:arrival") == "0");
                QVERIFY(secElem.tagValue("mx:departure").isEmpty() || secElem.tagValue("mx:departure") == "0");
            }
        }

        QVERIFY(model.departurePlatformRow() >= 0);
        QVERIFY(model.arrivalPlatformRow() >= 0);

        // fuzzy platform matching
        p.setName(QStringLiteral("10 D-F"));
        model.setArrivalPlatform(p);
        p.setName(QStringLiteral("9A-C"));
        model.setDeparturePlatform(p);
        QVERIFY(platformChangeSpy.wait());

        QVERIFY(model.departurePlatformRow() >= 0);
        auto idx = model.index(model.departurePlatformRow(), 0);
        QVERIFY(idx.isValid());
        QCOMPARE(model.rowCount(idx), 5);
        for (int i = 0; i < model.rowCount(idx); ++i) {
            const auto secElem = model.index(i, 0, idx).data(PlatformModel::ElementRole).value<OSM::Element>();
            QVERIFY(secElem.type() != OSM::Type::Null);
            QCOMPARE(secElem.tagValue("mx:departure"), i < 3 ? "1" : "0");
        }

        QVERIFY(model.arrivalPlatformRow() >= 0);
        idx = model.index(model.arrivalPlatformRow(), 0);
        QVERIFY(idx.isValid());
        QCOMPARE(model.rowCount(idx), 5);
        for (int i = 0; i < model.rowCount(idx); ++i) {
            const auto secElem = model.index(i, 0, idx).data(PlatformModel::ElementRole).value<OSM::Element>();
            QVERIFY(secElem.type() != OSM::Type::Null);
            QCOMPARE(secElem.tagValue("mx:arrival"), i >= 3 ? "1" : "0");
        }

        // non-matching platforms
        p.setName(QStringLiteral("13"));
        model.setArrivalPlatform(p);
        p.setName(QStringLiteral("14 A-D"));
        model.setDeparturePlatform(p);
        QVERIFY(platformChangeSpy.wait());
        QCOMPARE(model.departurePlatformRow(), -1);
        QCOMPARE(model.arrivalPlatformRow(), -1);
    }
};

QTEST_GUILESS_MAIN(PlatformModelTest)

#include "platformmodeltest.moc"
