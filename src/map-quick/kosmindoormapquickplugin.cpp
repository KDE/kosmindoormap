/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kosmindoormapquickplugin.h"

#include "osmaddress.h"
#include "osmelement.h"

#include <KOSMIndoorMap/MapData>
#include <KOSMIndoorMap/PlatformModel>

#include <QQmlEngine>

using namespace KOSMIndoorMap;

void KOSMIndoorMapQuickPlugin::registerTypes(const char *uri)
{
    Q_UNUSED(uri);
    Q_INIT_RESOURCE(assets);

    qRegisterMetaType<MapData>();
    qRegisterMetaType<OSMAddress>();
    qRegisterMetaType<OSMElement>();
    qRegisterMetaType<Platform>();
    qRegisterMetaType<Platform::Mode>();

    qmlRegisterUncreatableMetaObject(Platform::staticMetaObject, "org.kde.kosmindoormap", 1, 0, "Platform", {});
}

#include "moc_kosmindoormapquickplugin.cpp"
