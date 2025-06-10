/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kosmindoormapquickplugin.h"

#include "osmaddress.h"
#include "osmelement.h"
#include "platformutil.h"

#include <KOSMIndoorMap/MapData>

#include <QQmlEngine>

using namespace KOSMIndoorMap;

void KOSMIndoorMapQuickPlugin::registerTypes(const char *uri)
{
    Q_UNUSED(uri);
    Q_INIT_RESOURCE(assets);

    qRegisterMetaType<MapData>();
    qRegisterMetaType<OSMAddress>();
    qRegisterMetaType<OSMElement>();

    qmlRegisterSingletonType("org.kde.kosmindoormap", 1, 0, "PlatformUtil", [](QQmlEngine*, QJSEngine *engine) -> QJSValue {
        return engine->toScriptValue(KOSMIndoorMap::PlatformUtil());
    });
}

#include "moc_kosmindoormapquickplugin.cpp"
