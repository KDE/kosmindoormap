/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kosmindoormap-kpublictransport-integration.h"
#include "locationqueryoverlayproxymodel.h"
#include "realtimeequipmentmodel.h"

#include <QQmlEngine>

using namespace KOSMIndoorMap;

void KOSMIndoorMapQuickPlugin::registerTypes(const char *uri)
{
    Q_UNUSED(uri);
    qmlRegisterType<LocationQueryOverlayProxyModel>("org.kde.kosmindoormap.kpublictransport", 1, 0, "LocationQueryOverlayProxyModel");
    qmlRegisterType<RealtimeEquipmentModel>("org.kde.kosmindoormap.kpublictransport", 1, 0, "RealtimeEquipmentModel");
}

#include "moc_kosmindoormap-kpublictransport-integration.cpp"
