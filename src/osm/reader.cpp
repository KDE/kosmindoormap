/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "reader.h"
#include "ioplugin.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QPluginLoader>

using namespace OSM;

Q_IMPORT_PLUGIN(OSM_O5mIOPlugin)
Q_IMPORT_PLUGIN(OSM_PbfIOPlugin)
Q_IMPORT_PLUGIN(OSM_XmlIOPlugin)

IOPluginInterface::~IOPluginInterface() = default;

std::unique_ptr<AbstractReader> Reader::readerForFileName(QStringView fileName, OSM::DataSet *dataSet)
{
    const auto plugins = QPluginLoader::staticPlugins();
    for (const auto &plugin : plugins) {
        const auto md = plugin.metaData();
        if (md.value(QLatin1String("IID")).toString() != QLatin1String(OSMIOPluginInteraface_iid)) {
            continue;
        }
        const auto exts = md.value(QLatin1String("MetaData")).toObject().value(QLatin1String("fileExtensions")).toArray();
        for (const auto &ext : exts) {
            if (fileName.endsWith(ext.toString(), Qt::CaseInsensitive)) {
                return qobject_cast<IOPluginInterface*>(plugin.instance())->createReader(dataSet);
            }
        }
    }

    return {};
}
