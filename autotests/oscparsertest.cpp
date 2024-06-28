/*
    SPDX-FileCopyrightText: 2024 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <osm/abstractreader.h>
#include <osm/datatypes.h>
#include <osm/io.h>

#include <QTest>

using namespace Qt::Literals::StringLiterals;

class OscParserTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testChangesetLoad()
    {
        OSM::DataSet dataSet;

        {
            auto p = OSM::IO::readerForMimeType(u"application/vnd.openstreetmap.data+xml", &dataSet);
            QVERIFY(p);
            QFile baseFile(QStringLiteral(SOURCE_DIR "/data/changeset/base.osm"));
            QVERIFY(baseFile.open(QFile::ReadOnly));
            p->read(&baseFile);
            QVERIFY(!p->hasError());
        }
        QCOMPARE(dataSet.ways.size(), 1);
        QCOMPARE(dataSet.nodes.size(), 10);

        {
            auto p = OSM::IO::readerForMimeType(u"application/vnd.openstreetmap.changes+xml", &dataSet);
            QVERIFY(p);
            QFile changeFile(QStringLiteral(SOURCE_DIR "/data/changeset/changeset.osc"));
            QVERIFY(changeFile.open(QFile::ReadOnly));
            p->read(&changeFile);
            QVERIFY(!p->hasError());
        }
        QCOMPARE(dataSet.ways.size(), 1);
        QCOMPARE(dataSet.nodes.size(), 11);
        QCOMPARE(OSM::tagValue(dataSet.nodes[0], "name"), "Coffee Bar");

        auto modifiedNode = dataSet.node(1237008670);
        QVERIFY(modifiedNode);
        QCOMPARE(OSM::tagValue(*modifiedNode, "access"), "private");
    }
};

QTEST_GUILESS_MAIN(OscParserTest)

#include "oscparsertest.moc"
