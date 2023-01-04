/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <overpassquery.h>
#include <overpassquerymanager.h>
#include <io.h>

#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDebug>
#include <QFile>
#include <QJsonDocument>

#include <iostream>

#define S(x) QStringLiteral(x)

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    QCommandLineParser parser;
    parser.setApplicationDescription(S("overpassql-cli"));
    parser.addHelpOption();
    QCommandLineOption queryOption({ S("q"), S("query") }, S("Overpass QL query to run"), S("query-string"));
    parser.addOption(queryOption);
    QCommandLineOption queryFileOption({ S("f"), S("query-file") }, S("File to read Overpass QL query from"), S("query-file"));
    parser.addOption(queryFileOption);
    QCommandLineOption bboxOption({ S("b"), S("bbox") }, S("Query bounding box"), S("x,y,w,h"));
    parser.addOption(bboxOption);
    QCommandLineOption tileSizeOption({ S("t"), S("tile-size") }, S("Query tile size"), S("w,h"));
    parser.addOption(tileSizeOption);
    QCommandLineOption minTileSizeOption({ S("m"), S("minimum-tile-size") }, S("Minimum query tile size"), S("w,h"));
    parser.addOption(minTileSizeOption);
    QCommandLineOption outFileOption( { S("o"), S("output") }, S("Output file name"), S("out"));
    parser.addOption(outFileOption);
    parser.process(app);

    OSM::OverpassQueryManager mgr;
    OSM::OverpassQuery query;

    if (parser.isSet(queryFileOption)) {
        QFile f(parser.value(queryFileOption));
        if (!f.open(QFile::ReadOnly)) {
            std::cerr << "failed to read query file: " << qPrintable(f.errorString()) << std::endl;
            return 1;
        }
        query.setQuery(QString::fromUtf8(f.readAll()));
    } else {
        query.setQuery(parser.value(queryOption));
    }

    if (parser.isSet(bboxOption)) {
        const auto s = parser.value(bboxOption).split(QLatin1Char(','));
        if (s.size() != 4) {
            std::cerr << "invalid bbox format" << std::endl;
            return 1;
        }
        QRectF bbox(s[0].toDouble(), s[1].toDouble(), s[2].toDouble(), s[3].toDouble());
        query.setBoundingBox(bbox);
    }

    if (parser.isSet(tileSizeOption)) {
        const auto s = parser.value(tileSizeOption).split(QLatin1Char(','));
        if (s.size() != 2) {
            std::cerr << "invalid tile size format" << std::endl;
            return 1;
        }
        QSizeF tileSize(s[0].toDouble(), s[1].toDouble());
        query.setTileSize(tileSize);
    }
    if (parser.isSet(minTileSizeOption)) {
        const auto s = parser.value(minTileSizeOption).split(QLatin1Char(','));
        if (s.size() != 2) {
            std::cerr << "invalid minimum tile size format" << std::endl;
            return 1;
        }
        QSizeF minTileSize(s[0].toDouble(), s[1].toDouble());
        query.setMinimumTileSize(minTileSize);
    }

    QObject::connect(&query, &OSM::OverpassQuery::finished, [&]() {
        if (query.error() != OSM::OverpassQuery::NoError) {
            std::cerr << "query error" << std::endl;
            app.exit(1);
        }

        const auto outFileName = parser.value(outFileOption);
        QFile outFile(outFileName);
        std::unique_ptr<OSM::AbstractWriter> writer;
        if (!outFileName.isEmpty() && outFile.open(QFile::WriteOnly)) {
            writer = OSM::IO::writerForFileName(outFile.fileName());
        } else if (outFile.open(stdout, QFile::WriteOnly)) {
            writer = OSM::IO::writerForMimeType(u"vnd.openstreetmap.data+xml");
        }
        if (!writer) {
            std::cerr << "unsupported output file format" << std::endl;
            app.exit(1);
        }
        writer->write(query.result(), &outFile);

        app.quit();
    });
    mgr.execute(&query);

    return app.exec();
}
