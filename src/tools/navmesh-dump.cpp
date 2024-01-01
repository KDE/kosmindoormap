/*
    SPDX-FileCopyrightText: 2024 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <KOSMIndoorRouting/NavMeshBuilder>

#include <KOSMIndoorMap/EquipmentModel>
#include <KOSMIndoorMap/MapData>
#include <KOSMIndoorMap/MapLoader>

#include <osm/datatypes.h>
#include <osm/io.h>

#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDebug>
#include <QFile>

using namespace KOSMIndoorMap;
using namespace KOSMIndoorRouting;

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    QCommandLineParser parser;
    QCommandLineOption pointOpt({QStringLiteral("p"), QStringLiteral("point")}, QStringLiteral("download area around point"), QStringLiteral("lat,lon"));
    parser.addOption(pointOpt);
    QCommandLineOption outputOpt({QStringLiteral("o"), QStringLiteral("output")}, QStringLiteral("output path"), QStringLiteral("directory"));
    parser.addOption(outputOpt);
    QCommandLineOption nameOpt({QStringLiteral("n"), QStringLiteral("name")}, QStringLiteral("output name"), QStringLiteral("name"));
    parser.addOption(nameOpt);
    parser.addHelpOption();
    parser.process(app);

    const auto coords = QStringView(parser.value(pointOpt)).split(QLatin1Char(','));
    if (coords.size() != 2) {
        qCritical() << "Invalid coordinate!";
        return 1;
    }
    OSM::Coordinate coord{coords[0].toDouble(), coords[1].toDouble()};

    MapLoader loader;
    loader.loadForCoordinate(coord.latF(), coord.lonF());

    QObject::connect(&loader, &MapLoader::done, &app, &QCoreApplication::quit);
    QCoreApplication::exec();
    auto data = loader.takeData();

    // TODO use RealtimeEquipmentModel
    KOSMIndoorMap::EquipmentModel equipmentModel;
    equipmentModel.setMapData(data);

    KOSMIndoorRouting::NavMeshBuilder navMeshBuilder;
    navMeshBuilder.setMapData(data);
    navMeshBuilder.setEquipmentModel(&equipmentModel);
    navMeshBuilder.writeDebugNavMesh(parser.value(outputOpt) + QLatin1Char('/') + parser.value(nameOpt) + QLatin1String(".gset"), parser.value(outputOpt) + QLatin1Char('/') + parser.value(nameOpt) + QLatin1String(".obj"));
    navMeshBuilder.start();
}
