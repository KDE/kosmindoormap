/*
    SPDX-FileCopyrightText: 2024 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KOSMINDOORMAP_USERAGENT_P_H
#define KOSMINDOORMAP_USERAGENT_P_H

class QByteArray;

namespace KOSMIndoorMap {
    /** HTTP User-Agent header to use for requests from this application. */
    [[nodiscard]] QByteArray userAgent();
}

#endif
