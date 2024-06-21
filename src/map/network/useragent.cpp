/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "useragent_p.h"

#include <QCoreApplication>

using namespace Qt::Literals::StringLiterals;

QByteArray KOSMIndoorMap::userAgent()
{
    return (QCoreApplication::applicationName() + '/'_L1 + QCoreApplication::applicationVersion()).toUtf8();
}
