/*
    SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <osm/datatypes.h>
#include <osm/element.h>
#include <osm/io.h>
#include <osm/languages.h>

#include <QFile>
#include <QLocale>
#include <QTest>
#include <QtPlugin>

QDebug operator<<(QDebug debug, const OSM::Languages &langs)
{
    QStringList l;
    std::transform(langs.languages.begin(), langs.languages.end(), std::back_inserter(l), [](const std::string &s) { return QString::fromStdString(s); });
    debug << l;
    return debug;
}

Q_IMPORT_PLUGIN(OSM_XmlIOPlugin)

class LocalizedTagTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testLocalizedLookup()
    {
        QFile f(QStringLiteral(SOURCE_DIR "/data/localizedtags.xml"));
        QVERIFY(f.open(QFile::ReadOnly));

        OSM::DataSet dataSet;
        auto p = OSM::IO::readerForFileName(f.fileName(), &dataSet);
        p->read(&f);
        QCOMPARE(dataSet.nodes.size(), 1);

        QLocale l(QLocale::English, QLocale::LatinScript, QLocale::UnitedStates);
        auto langs = OSM::Languages::fromQLocale(l);

        const OSM::Element e(&dataSet.nodes[0]);
        QCOMPARE(e.tagValue(langs, "name"), "english");
        QCOMPARE(e.tagValue(langs, "name:disused"), "subtag english");
        QCOMPARE(e.tagValue(langs, "name:local"), "no local english");

        l = QLocale(QLocale::Korean, QLocale::HanScript, QLocale::SouthKorea);
        langs = OSM::Languages::fromQLocale(l);
        QCOMPARE(e.tagValue(langs, "name"), "korean");
        QCOMPARE(e.tagValue(langs, "name:disused"), "subtag local");
        QVERIFY(e.tagValue(langs, "name:local").startsWith("no local "));

        l = QLocale(QStringLiteral("sr-Latn-RS"));
        langs = OSM::Languages::fromQLocale(l);
        qDebug() << l.uiLanguages() << langs;
        QCOMPARE(e.tagValue(langs, "name"), "romanized serbian");
        QCOMPARE(e.tagValue(langs, "name:disused"), "subtag serbian");

        l = QLocale(QStringLiteral("ja-JP"));
        langs = OSM::Languages::fromQLocale(l);
        QCOMPARE(e.tagValue(langs, "name"), "local");
        QCOMPARE(e.tagValue(langs, "name:disused"), "subtag local");
    }
};

QTEST_GUILESS_MAIN(LocalizedTagTest)

#include "localizedtagtest.moc"
