/*
    SPDX-FileCopyrightText: 2024 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KOSMINDOORROUTING_TYPES_H
#define KOSMINDOORROUTING_TYPES_H

#include <KOSMIndoorRouting/RoutingProfile>

#include <QQmlEngine>

struct RoutingProfileForeign {
    Q_GADGET
    QML_VALUE_TYPE(routingProfile)
    QML_FOREIGN(KOSMIndoorRouting::RoutingProfile)
    QML_UNCREATABLE("only provided via C++ API")
};

#endif
