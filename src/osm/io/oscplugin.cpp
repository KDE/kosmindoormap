/*
    SPDX-FileCopyrightText: 2024 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#define QT_STATICPLUGIN 1

#include "../ioplugin.h"
#include "../oscparser.h"

class OSM_OscIOPlugin : public QObject, public OSM::IOPlugin<OSM::OscParser>
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID OSMIOPluginInteraface_iid FILE "oscplugin.json")
    Q_INTERFACES(OSM::IOPluginInterface)
};

#include "oscplugin.moc"
