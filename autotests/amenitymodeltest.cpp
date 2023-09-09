/*
    SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "../src/map-quick/amenitymodel.h"

#include <KOSMIndoorMap/MapData>

#include <osm/io.h>

#include <QAbstractItemModelTester>
#include <QFile>
#include <QTest>
#include <QtPlugin>

Q_IMPORT_PLUGIN(OSM_XmlIOPlugin)

using namespace KOSMIndoorMap;

void initLocale()
{
    qputenv("LC_ALL", "en_US.utf-8");
    qputenv("TZ", "UTC");

    Q_INIT_RESOURCE(assets);
}

Q_CONSTRUCTOR_FUNCTION(initLocale)

class AmenityModelTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testModel()
    {
        // verify the locale matches what the test data expects
        // this is a workaround for test failures on OBS
        if (QLocale().createSeparatedList({QStringLiteral("A"), QStringLiteral("B")}) != QLatin1String("A and B")) {
            QSKIP("locale doesn't behave as expected!");
        }

        const QString osmFile = QStringLiteral(SOURCE_DIR "/data/amenitymodel/amenitymodeltest.osm");
        QFile inFile(osmFile);
        QVERIFY(inFile.open(QFile::ReadOnly));

        OSM::DataSet dataSet;
        auto p = OSM::IO::readerForFileName(osmFile, &dataSet);
        p->read(&inFile);
        QVERIFY(!dataSet.nodes.empty());

        MapData mapData;
        mapData.setDataSet(std::move(dataSet));
        QVERIFY(!mapData.isEmpty());

        AmenityModel model;
        QAbstractItemModelTester modelTest(&model);
        model.setMapData(mapData);

        QCOMPARE(model.rowCount(), 4);

        auto idx = model.index(0, 0);
        QCOMPARE(idx.data(AmenityModel::NameRole).toString(), QLatin1String("Burger Place"));
        QCOMPARE(idx.data(AmenityModel::TypeNameRole).toString(), QLatin1String("Fast Food"));
        QCOMPARE(idx.data(AmenityModel::CuisineRole).toString(), QLatin1String("Burger"));
        QCOMPARE(idx.data(AmenityModel::GroupRole).toInt(), AmenityModel::FoodGroup);
        QVERIFY(idx.data(AmenityModel::IconSourceRole).toString().contains(QLatin1String("fast_food.svg")));

        idx = model.index(1, 0);
        QCOMPARE(idx.data(AmenityModel::NameRole).toString(), QLatin1String("My Supermarket"));
        QCOMPARE(idx.data(AmenityModel::TypeNameRole).toString(), QLatin1String("Supermarket"));
        QVERIFY(idx.data(AmenityModel::CuisineRole).toString().isEmpty());
        QCOMPARE(idx.data(AmenityModel::GroupRole).toInt(), AmenityModel::ShopGroup);
        QVERIFY(!idx.data(AmenityModel::OpeningHoursRole).toString().isEmpty());
        QVERIFY(idx.data(AmenityModel::IconSourceRole).toString().contains(QLatin1String("supermarket.svg")));

        idx = model.index(2, 0);
        QVERIFY(idx.data(AmenityModel::NameRole).toString().isEmpty());
        QCOMPARE(idx.data(AmenityModel::TypeNameRole).toString(), QLatin1String("Toilets"));
        QVERIFY(idx.data(AmenityModel::CuisineRole).toString().isEmpty());
        QCOMPARE(idx.data(AmenityModel::GroupRole).toInt(), AmenityModel::ToiletGroup);

        idx = model.index(3, 0);
        QVERIFY(idx.data(AmenityModel::NameRole).toString().isEmpty());
        QCOMPARE(idx.data(AmenityModel::FallbackNameRole).toString(), QLatin1String("Some Company"));
        QCOMPARE(idx.data(AmenityModel::TypeNameRole).toString(), QLatin1String("Car Rental"));
        QVERIFY(idx.data(AmenityModel::CuisineRole).toString().isEmpty());
        QCOMPARE(idx.data(AmenityModel::GroupRole).toInt(), AmenityModel::AmenityGroup);
    }
};

QTEST_GUILESS_MAIN(AmenityModelTest)

#include "amenitymodeltest.moc"
