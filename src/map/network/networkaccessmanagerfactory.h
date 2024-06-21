/*
    SPDX-FileCopyrightText: 2024 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KOSMINDOORMAP_NETWORKACCESSMANAGERFACTORY_H
#define KOSMINDOORMAP_NETWORKACCESSMANAGERFACTORY_H

#include "kosmindoormap_export.h"

#include <functional>

class QNetworkAccessManager;

namespace KOSMIndoorMap {

/** Network access manager factory. */
using NetworkAccessManagerFactory = std::function<QNetworkAccessManager *()>;

/** Default implementation if not using an application-specific
 *  QNetworkAccessManager instance.
 */
KOSMINDOORMAP_EXPORT QNetworkAccessManager* defaultNetworkAccessManagerFactory();

}

#endif
