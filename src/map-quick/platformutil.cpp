/*
    SPDX-FileCopyrightText: ⓒ 2025 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "platformutil.h"

#include <KLocalizedString>

using namespace KOSMIndoorMap;

QString PlatformUtil::modeName(Platform::Mode mode) {
    switch (mode) {
        case Platform::Unknown: return {};
        case Platform::Rail: return i18nc("mode of transport", "Railway");
        case Platform::LightRail: return i18nc("mode of transport", "Light Rail");
        case Platform::Subway: return i18nc("mode of transport", "Subway");
        case Platform::Tram: return i18nc("mode of transport", "Tram");
        case Platform::Monorail: return i18nc("mode of transport", "Monorail");
        case Platform::Bus: return i18nc("mode of transport", "Bus");
    }

    return {};
}
