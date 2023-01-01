/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "reader.h"

#include "o5mparser.h"
#include "osmpbfparser.h"
#include "xmlparser.h"

using namespace OSM;

std::unique_ptr<AbstractReader> Reader::readerForFileName(QStringView fileName, OSM::DataSet *dataSet)
{
    if (fileName.endsWith(QLatin1String(".o5m"))) {
        return std::make_unique<O5mParser>(dataSet);
    }
    if (fileName.endsWith(QLatin1String(".pbf"))) {
        return std::make_unique<OsmPbfParser>(dataSet);
    }
    if (fileName.endsWith(QLatin1String(".osm")) || fileName.endsWith(QLatin1String(".xml"))) {
        return std::make_unique<XmlParser>(dataSet);
    }

    return {};
}
