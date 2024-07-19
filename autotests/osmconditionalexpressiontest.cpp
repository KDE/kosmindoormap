/*
    SPDX-FileCopyrightText: 2024 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "../src/map/content/osmconditionalexpression.cpp"
#include "../src/map/scene/openinghourscache.cpp"

#include <osm/io.h>

#include <QFile>
#include <QTest>

using namespace Qt::Literals;

class OSMConditionalExpressionTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testEntireRange()
    {
        const QString osmFile = QStringLiteral(SOURCE_DIR "/data/changeset/base.osm");
        QFile inFile(osmFile);
        QVERIFY(inFile.open(QFile::ReadOnly));

        OSM::DataSet dataSet;
        auto p = OSM::IO::readerForFileName(osmFile, &dataSet);
        p->read(&inFile);
        QVERIFY(!dataSet.nodes.empty());

        MapData mapData;
        mapData.setDataSet(std::move(dataSet));
        QVERIFY(!mapData.isEmpty());

        OpeningHoursCache cache;
        cache.setMapData(mapData);
        cache.setTimeRange({{2024, 7, 20}, {}}, {{2024, 7, 22}, {}});

        OSMConditionalExpressionContext context;
        context.element = mapData.dataSet().node(1237008670);

        {
            OSMConditionalExpression expr;
            expr.parse("option 1 @ Mo-Fr; option 2 @ Sa-Su");
            context.openingHoursCache = &cache;
            QCOMPARE(expr.evaluate(context), "option 2");
            QCOMPARE(expr.evaluate(context), "option 2");
        }

        {
            OSMConditionalExpression expr;
            expr.parse("option 1 @ Fr-Mo; option 2 @ Sa-Su;PH; option 3 2022-2023");
            context.openingHoursCache = &cache;
            QCOMPARE(expr.evaluate(context), "option 1");
            QCOMPARE(expr.evaluate(context), "option 1");
        }

        {
            OSMConditionalExpression expr;
            expr.parse("talks @ Sa-Su");
            context.openingHoursCache = &cache;
            QCOMPARE(expr.evaluate(context), "talks");
            QCOMPARE(expr.evaluate(context), "talks");
        }
    }
};

QTEST_GUILESS_MAIN(OSMConditionalExpressionTest)

#include "osmconditionalexpressiontest.moc"
