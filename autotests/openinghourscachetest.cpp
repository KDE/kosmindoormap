/*
    SPDX-FileCopyrightText: 2024 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "../src/map/scene/openinghourscache.cpp"

#include <osm/datatypes.h>
#include <osm/io.h>

#include <QFile>
#include <QTest>

using namespace Qt::Literals;

class OpeningHoursCacheTest : public QObject
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
        cache.setTimeRange({{2024, 7, 20}, {}}, {{2024, 7, 21}, {23, 59}});

        OSM::Element elem(mapData.dataSet().node(1237008670));
        QVERIFY(elem);
        QVERIFY(cache.isEntirelyClosedInRange(elem, "Mo-Fr"));
        QVERIFY(cache.isEntirelyClosedInRange(elem, "Mo-Fr"));
        QVERIFY(!cache.isEntirelyClosedInRange(elem, "Fr-Sa"));
        QVERIFY(!cache.isEntirelyClosedInRange(elem, "Fr-Sa"));

        QVERIFY(!cache.isAtCurrentTime(elem, "Mo-Fr"));
        QVERIFY(!cache.isAtCurrentTime(elem, "Mo-Fr"));
        QVERIFY(cache.isEntirelyClosedInRange(elem, "Mo-Fr"));

        QVERIFY(cache.isAtCurrentTime(elem, "Sa-Su"));
        QVERIFY(cache.isAtCurrentTime(elem, "Sa-Su"));
    }
};

QTEST_GUILESS_MAIN(OpeningHoursCacheTest)

#include "openinghourscachetest.moc"
