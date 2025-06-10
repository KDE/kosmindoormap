/*
    SPDX-FileCopyrightText: ⓒ 2025 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KOSMINDOORMAP_PLATFORMUTIL_H
#define KOSMINDOORMAP_PLATFORMUTIL_H

#include <KOSMIndoorMap/Platform>

namespace KOSMIndoorMap {
/** Helper methods for dealing with public transport platform data. */
class PlatformUtil {
    Q_GADGET
public:
    Q_INVOKABLE [[nodiscard]] static QString modeName(KOSMIndoorMap::Platform::Mode mode);
};

};

#endif
