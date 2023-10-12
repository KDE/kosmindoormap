/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <KOSMIndoorMap/MapData>
#include <KOSMIndoorMap/MapLoader>
#include <loader/tilecache_p.h>

#include <osm/datatypes.h>
#include <osm/io.h>

#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDebug>
#include <QFile>
#include <QtPlugin>

#if HAVE_OSM_PBF_SUPPORT
Q_IMPORT_PLUGIN(OSM_PbfIOPlugin)
#endif
Q_IMPORT_PLUGIN(OSM_XmlIOPlugin)

using namespace KOSMIndoorMap;

static void filterByBbox(OSM::DataSet &dataSet, OSM::BoundingBox bbox)
{
    dataSet.relations.erase(std::remove_if(dataSet.relations.begin(), dataSet.relations.end(), [bbox](const auto &rel) {
        return !OSM::intersects(rel.bbox, bbox);
    }), dataSet.relations.end());
    dataSet.ways.erase(std::remove_if(dataSet.ways.begin(), dataSet.ways.end(), [bbox](const auto &way) {
        return !OSM::intersects(way.bbox, bbox);
    }), dataSet.ways.end());
    dataSet.nodes.erase(std::remove_if(dataSet.nodes.begin(), dataSet.nodes.end(), [bbox](const auto &nd) {
        return !OSM::contains(bbox, nd.coordinate);
    }), dataSet.nodes.end());
}

template <typename Elem>
static bool containsElement(const std::vector<Elem> &elems, OSM::Id id)
{
    const auto it = std::lower_bound(elems.begin(), elems.end(), id, [](const Elem &lhs, OSM::Id rhs) { return lhs.id < rhs; });
    return it != elems.end() && (*it).id == id;
}

static void purgeDanglingReferences(OSM::DataSet &dataSet)
{
    for (auto &rel : dataSet.relations) {
        rel.members.erase(std::remove_if(rel.members.begin(), rel.members.end(), [&dataSet](const auto &mem) {
            switch (mem.type()) {
                case OSM::Type::Null:
                    Q_UNREACHABLE();
                case OSM::Type::Node:
                    return !containsElement(dataSet.nodes, mem.id);
                case OSM::Type::Way:
                    return !containsElement(dataSet.ways, mem.id);
                case OSM::Type::Relation:
                    return !containsElement(dataSet.relations, mem.id);
            }
            return false;
        }), rel.members.end());
    }
}

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    QCommandLineParser parser;
    parser.addHelpOption();
    QCommandLineOption bboxOpt({QStringLiteral("b"), QStringLiteral("bbox")}, QStringLiteral("bounding box to download"), QStringLiteral("minlat,minlon,maxlat,maxlon"));
    parser.addOption(bboxOpt);
    QCommandLineOption clipOpt({QStringLiteral("c"), QStringLiteral("clip")}, QStringLiteral("clip to bounding box"));
    parser.addOption(clipOpt);
    QCommandLineOption outOpt({QStringLiteral("o"), QStringLiteral("out")}, QStringLiteral("output file"), QStringLiteral("file"));
    parser.addOption(outOpt);
    QCommandLineOption pointOpt({QStringLiteral("p"), QStringLiteral("point")}, QStringLiteral("download area around point"), QStringLiteral("lat,lon"));
    parser.addOption(pointOpt);
    QCommandLineOption tileOpt({QStringLiteral("t"), QStringLiteral("tile")}, QStringLiteral("download tile"), QStringLiteral("z/x/y"));
    parser.addOption(tileOpt);
    parser.process(app);

    if ((!parser.isSet(bboxOpt) && !parser.isSet(pointOpt) && !parser.isSet(tileOpt)) || !parser.isSet(outOpt)) {
        parser.showHelp(1);
        return 1;
    }

    OSM::BoundingBox bbox;
    MapLoader loader;
    if (parser.isSet(bboxOpt)) {
        const auto coords = QStringView(parser.value(bboxOpt)).split(QLatin1Char(','));
        if (coords.size() == 4) {
            bbox.min = OSM::Coordinate(coords[0].toDouble(), coords[1].toDouble());
            bbox.max = OSM::Coordinate(coords[2].toDouble(), coords[3].toDouble());
        }
        if (!bbox.isValid()) {
            qCritical() << "Invalid bounding box!";
            return 1;
        }
    }

    if (parser.isSet(pointOpt)) {
        const auto coords = QStringView(parser.value(pointOpt)).split(QLatin1Char(','));
        if (coords.size() != 2) {
            qCritical() << "Invalid coordinate!";
            return 1;
        }
        OSM::Coordinate coord{coords[0].toDouble(), coords[1].toDouble()};
        loader.loadForCoordinate(coord.latF(), coord.lonF());
    } else if (parser.isSet(tileOpt)) {
        const auto coords = QStringView(parser.value(tileOpt)).split(QLatin1Char('/'));
        if (coords.size() != 3) {
            qCritical() << "Invalid tile!";
            return 1;
        }
        Tile t(coords[1].toUInt(), coords[2].toUInt(), coords[0].toUInt());
        loader.loadForTile(t);
    } else if (parser.isSet(bboxOpt)) {
        loader.loadForBoundingBox(bbox);
    }

    QObject::connect(&loader, &MapLoader::done, &app, &QCoreApplication::quit);
    QCoreApplication::exec();
    auto data = loader.takeData();

    if (parser.isSet(clipOpt) && parser.isSet(bboxOpt)) {
        filterByBbox(data.dataSet(), bbox);
        purgeDanglingReferences(data.dataSet());
    }

    QFile f(parser.value(outOpt));
    if (!f.open(QFile::WriteOnly)) {
        qCritical() << f.errorString();
        return 1;
    }
    auto writer = OSM::IO::writerForFileName(f.fileName());
    if (!writer) {
        qCritical() << "no file writer for requested format:" << f.fileName();
        return 1;
    }
    writer->write(data.dataSet(), &f);
    return 0;
}
