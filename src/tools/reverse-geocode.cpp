/*
    SPDX-FileCopyrightText: 2025 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <KOSMIndoorMap/ReverseGeocodingJob>

#include <KOSM/Element>

#include <QCommandLineParser>
#include <QCoreApplication>

#include <iostream>

using namespace Qt::Literals;
using namespace KOSMIndoorMap;

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    QCommandLineParser parser;
    parser.addHelpOption();
    QCommandLineOption lonOpt({u"x"_s, u"lon"_s}, u"Longitude"_s, u"longitude"_s);
    parser.addOption(lonOpt);
    QCommandLineOption latOpt({u"y"_s, u"lat"_s}, u"Latitude"_s, u"latitude"_s);
    parser.addOption(latOpt);
    QCommandLineOption radiusOpt({u"r"_s, u"radius"_s}, u"Radius"_s, u"meter"_s);
    parser.addOption(radiusOpt);
    parser.process(app);

    if ((!parser.isSet(lonOpt) && !parser.isSet(latOpt) && !parser.isSet(radiusOpt))) {
        parser.showHelp(1);
        return 1;
    }

    ReverseGeocodingJob job;
    QObject::connect(&job, &ReverseGeocodingJob::finished, &app, &QCoreApplication::quit);
    job.setCoordinate(parser.value(latOpt).toDouble(), parser.value(lonOpt).toDouble());
    job.setRadius(parser.value(radiusOpt).toDouble());
    job.start();
    QCoreApplication::exec();

    if (job.hasError()) {
        std::cerr << qPrintable(job.errorMessage()) << std::endl;
        return 1;
    }

    for (const auto &elem : job.result()) {
        std::cout << qPrintable(elem.url()) << std::endl;
        for (auto it = elem.tagsBegin(); it != elem.tagsEnd(); ++it) {
            std::cout << "    " << (*it).key.name() << ": " << (*it).value.constData() << std::endl;
        }
    }

    return 0;
}
