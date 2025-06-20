/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <map/content/platformfinder_p.h>

#include <KOSMIndoorMap/MapData>
#include <KOSMIndoorMap/MapLoader>

#include <QMetaEnum>
#include <QProcess>
#include <QSignalSpy>
#include <QTest>

using namespace KOSMIndoorMap;

class PlatformFinderTest : public QObject
{
    Q_OBJECT
private:
    void writeElement(QIODevice *device, OSM::Element elem)
    {
        switch (elem.type()) {
            case OSM::Type::Null:
                device->write("<null>");
                break;
            case OSM::Type::Node:
                device->write("n" + QByteArray::number((qlonglong)elem.id()));
                break;
            case OSM::Type::Way:
                if (elem.id() >= 0) {
                    device->write("w" + QByteArray::number((qlonglong)elem.id()));
                } else {
                    device->write("<synthetic way>");
                }
                break;
            case OSM::Type::Relation:
                if (elem.id() >= 0) {
                    device->write("r" + QByteArray::number((qlonglong)elem.id()));
                } else {
                    device->write("<synthetic relation>");
                }
                break;
        }
    }
    void writeElement(QIODevice *device, const std::vector<OSM::Element> elems)
    {
        if (elems.size() == 1) {
            writeElement(device, elems[0]);
            return;
        }

        device->write("[");
        for (auto e : elems) {
            writeElement(device, e);
            device->write(" ");
        }
        device->write("]");
    }

private Q_SLOTS:
    void initTestCase()
    {
        QLocale::setDefault(QLocale(QLocale::English, QLocale::LatinScript, QLocale::UnitedStates));
    }

    void testPlatformFinder_data()
    {
        QTest::addColumn<QString>("input");
        QTest::addColumn<QString>("expected");

        // has platform_edge and platform section tags
        QTest::newRow("hamburg-altona") << (SOURCE_DIR "/data/platforms/hamburg-altona.osm") << (SOURCE_DIR "/data/platforms/hamburg-altona.platforms");
        // has no platform edges, but platform sections
        QTest::newRow("cologne-central") << (SOURCE_DIR "/data/platforms/cologne-central.osm") << (SOURCE_DIR "/data/platforms/cologne-central.platforms");
        // numberic and non-numeric platform names, free-floating sections with a non-standard encoding, no edges, partly no stop points
        QTest::newRow("paris-gare-de-lyon") << (SOURCE_DIR "/data/platforms/paris-gare-de-lyon.osm") << (SOURCE_DIR "/data/platforms/paris-gare-de-lyon.platforms");
        // multiple modes, 3 levels, no sections, multiple stop points on the same track
        QTest::newRow("berlin-central") << (SOURCE_DIR "/data/platforms/berlin-central.osm") << (SOURCE_DIR "/data/platforms/berlin-central.platforms");
        // split areas/edges, edges on the wrong level, triple platforms
        QTest::newRow("leipzig-central") << (SOURCE_DIR "/data/platforms/leipzig-central.osm") << (SOURCE_DIR "/data/platforms/leipzig-central.platforms");
        // multiple stop points per track, no edges, up to 4 split tracks
        QTest::newRow("hamburg-central") << (SOURCE_DIR "/data/platforms/hamburg-central.osm") << (SOURCE_DIR "/data/platforms/hamburg-central.platforms");
        // free-floating standard-compliant section markers
        QTest::newRow("aachen-central") << (SOURCE_DIR "/data/platforms/aachen-central.osm") << (SOURCE_DIR "/data/platforms/aachen-central.platforms");
        // alternative tram tagging
        QTest::newRow("wien-meidling") << (SOURCE_DIR "/data/platforms/wien-meidling.osm") << (SOURCE_DIR "/data/platforms/wien-meidling.platforms");
    }

    void testPlatformFinder()
    {
        QFETCH(QString, input);
        QFETCH(QString, expected);

        MapLoader loader;
        QSignalSpy doneSpy(&loader, &MapLoader::done);
        loader.loadFromFile(input);
        QVERIFY(doneSpy.wait());
        QCOMPARE(loader.isLoading(), false);
        QCOMPARE(loader.hasError(), false);

        const auto mapData = loader.takeData();
        QVERIFY(!mapData.dataSet().nodes.empty());
        QVERIFY(!mapData.dataSet().ways.empty());
        QVERIFY(!mapData.dataSet().relations.empty());
        QVERIFY(mapData.boundingBox().height() > 0);
        QVERIFY(mapData.boundingBox().width() > 0);

        PlatformFinder finder;
        const auto result = finder.find(mapData);
        QVERIFY(!result.empty());

        QFile outFile(QFileInfo(expected).fileName());
        QVERIFY(outFile.open(QFile::ReadWrite | QFile::Truncate | QFile::Text));

        for (const auto &platform : result) {
            outFile.write(platform.name().toUtf8() + "\n");
            outFile.write("  position: " + QByteArray::number(platform.position().latitude) + " " + QByteArray::number(platform.position().longitude) + "\n");
            outFile.write("  stop point: ");
            writeElement(&outFile, platform.stopPoint());
            outFile.write("\n");
            outFile.write("  edge: ");
            writeElement(&outFile, platform.edge());
            outFile.write("\n");
            outFile.write("  area: ");
            writeElement(&outFile, platform.area());
            outFile.write("\n");
            outFile.write("  track: ");
            writeElement(&outFile, platform.track());
            outFile.write("\n");
            outFile.write("  level: " + QByteArray::number(platform.level()) + "\n");
            outFile.write(QByteArray("  mode: ") + Platform::staticMetaObject.enumerator(0).valueToKey(platform.mode()) + "\n");
            if (!platform.lines().empty()) {
                outFile.write("  lines: " + platform.lines().join(QLatin1Char('|')).toUtf8() + "\n");
            }
            if (!platform.sections().empty()) {
                outFile.write("  sections:\n");
                for (const auto &section : platform.sections()) {
                    outFile.write("    name: " + section.name().toUtf8() + "\n");
                    outFile.write("    position: ");
                    writeElement(&outFile, section.position());
                    outFile.write("\n");
                }
            }
        }
        outFile.seek(0);
        const auto platforms = outFile.readAll();

        QFile expectedFile(expected);
        QVERIFY(expectedFile.open(QFile::ReadOnly | QFile::Text));
        const auto expectedPlatforms = expectedFile.readAll();

        if (platforms != expectedPlatforms) {
            QProcess proc;
            proc.setProcessChannelMode(QProcess::ForwardedChannels);
            proc.start(QStringLiteral("diff"), {QStringLiteral("-u"), expectedFile.fileName(), outFile.fileName()});
            proc.waitForFinished();
        }
        QVERIFY(platforms == expectedPlatforms);
    }
};

QTEST_GUILESS_MAIN(PlatformFinderTest)

#include "platformfindertest.moc"
