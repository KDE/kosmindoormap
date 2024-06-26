/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "../src/map-quick/osmelementinformationmodel.h"

#include <osm/io.h>

#include <QAbstractItemModelTester>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QProcess>
#include <QTest>

using namespace KOSMIndoorMap;

void initLocale()
{
    qputenv("LC_ALL", "en_US.utf-8");
    qputenv("TZ", "UTC");
}

Q_CONSTRUCTOR_FUNCTION(initLocale)

class OSMElementInfoModelTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testModel_data()
    {
        QTest::addColumn<QString>("osmFile");
        QTest::addColumn<QString>("modelFile");

        QDir dir(QStringLiteral(SOURCE_DIR "/data/osminfomodel/"));
        const auto lst = dir.entryList(QStringList(QStringLiteral("*.xml")), QDir::Files | QDir::Readable | QDir::NoSymLinks);
        for (const auto &file : lst) {
            const auto base = file.left(file.size() - 4);
            QTest::newRow(base.toLatin1().constData()) << (dir.path() + QLatin1Char('/') + file) << (dir.path() + QLatin1Char('/') + base + QLatin1String(".json"));
        }
    }

    void testModel()
    {
        // verify the locale matches what the test data expects
        // this is a workaround for test failures on OBS
        if (QLocale().createSeparatedList({QStringLiteral("A"), QStringLiteral("B")}) != QLatin1String("A and B")) {
            QSKIP("locale doesn't behave as expected!");
        }

        QFETCH(QString, osmFile);
        QFETCH(QString, modelFile);

        QFile inFile(osmFile);
        QVERIFY(inFile.open(QFile::ReadOnly));

        OSM::DataSet dataSet;
        auto p = OSM::IO::readerForFileName(osmFile, &dataSet);
        p->read(&inFile);
        QCOMPARE(dataSet.nodes.size(), 1);

        OSMElementInformationModel model;
        QAbstractItemModelTester modelTest(&model);
        model.setElement(OSMElement(OSM::Element(&dataSet.nodes[0])));

        QJsonObject top;
        top.insert(QStringLiteral("name"), model.name());
        if (!model.category().isEmpty()) {
            top.insert(QStringLiteral("category"), model.category());
        }
        QJsonArray modelContent;
        for (int row = 0; row < model.rowCount(); ++row) {
            const auto idx = model.index(row);
            QJsonObject modelRow;
            for (auto role : {OSMElementInformationModel::KeyLabelRole, OSMElementInformationModel::ValueRole, OSMElementInformationModel::ValueUrlRole, OSMElementInformationModel::CategoryLabelRole}) {
                if (!idx.data(role).toString().isEmpty()) {
                    modelRow.insert(QString::fromUtf8(model.roleNames().value(role)), idx.data(role).toString());
                }
            }
            modelContent.push_back(modelRow);
        }
        top.insert(QStringLiteral("content"), modelContent);

        model.clear();
        QCOMPARE(model.rowCount(), 0);

        QFile refFile(modelFile);
        QVERIFY(refFile.open(QFile::ReadOnly));
        const auto refContent = QJsonDocument::fromJson(refFile.readAll()).object();
        if (top != refContent) {
            QFile failFile(modelFile + QLatin1String(".fail"));
            QVERIFY(failFile.open(QFile::WriteOnly));
            failFile.write(QJsonDocument(top).toJson());
            failFile.close();

            QProcess proc;
            proc.setProcessChannelMode(QProcess::ForwardedChannels);
            proc.start(QStringLiteral("diff"), {QStringLiteral("-u"), modelFile, failFile.fileName()});
            QVERIFY(proc.waitForFinished());
        }
        QCOMPARE(top, refContent);
    }
};

QTEST_GUILESS_MAIN(OSMElementInfoModelTest)

#include "osmelementinfomodeltest.moc"
