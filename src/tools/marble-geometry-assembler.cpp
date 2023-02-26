/*
    SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <../src/map/loader/marblegeometryassembler_p.h>

#include <osm/datasetmergebuffer.h>
#include <osm/datatypes.h>
#include <osm/io.h>

#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDebug>
#include <QFile>
#include <QtPlugin>

Q_IMPORT_PLUGIN(OSM_XmlIOPlugin)

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    QCommandLineParser parser;
    parser.addHelpOption();
    QCommandLineOption outOpt({QStringLiteral("o"), QStringLiteral("output")}, QStringLiteral("output file"), QStringLiteral("file"));
    parser.addOption(outOpt);
    parser.process(app);

    const auto fileNames = parser.positionalArguments();
    if (fileNames.isEmpty()) {
        parser.showHelp(1);
    }

    OSM::DataSet dataSet;
    OSM::DataSetMergeBuffer mergeBuffer;
    KOSMIndoorMap::MarbleGeometryAssembler marbleMerger;
    marbleMerger.setDataSet(&dataSet);
    auto reader = OSM::IO::readerForMimeType(u"application/vnd.openstreetmap.data+o5m", &dataSet);
    reader->setMergeBuffer(&mergeBuffer);

    for (const auto &fileName : fileNames) {
        QFile f(fileName);
        if (!f.open(QFile::ReadOnly)) {
            qCritical() << f.fileName() << f.errorString();
            return 1;
        }
        const auto data = f.map(0, f.size());
        reader->read(data, f.size());
        marbleMerger.merge(&mergeBuffer);
    }

    marbleMerger.finalize();

    QFile outputFile;
    std::unique_ptr<OSM::AbstractWriter> writer;
    if (parser.isSet(outOpt)) {
        outputFile.setFileName(parser.value(outOpt));
        outputFile.open(QFile::WriteOnly);
        writer = OSM::IO::writerForFileName(outputFile.fileName());
    } else {
        outputFile.open(stdout, QFile::WriteOnly);
        writer = OSM::IO::writerForMimeType(u"application/vnd.openstreetmap.data+xml");
    }
    if (!outputFile.isOpen()) {
        qCritical() << outputFile.errorString();
        return 1;
    }

    if (!writer) {
        qCritical() << "no file writer for requested format:" << outputFile.fileName();
        return 1;
    }
    writer->write(dataSet, &outputFile);
    return 0;
}
