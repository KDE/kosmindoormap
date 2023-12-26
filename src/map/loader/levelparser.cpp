/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "levelparser_p.h"
#include "logging.h"

#include <osm/element.h>

// ### ugly workaround for locale-ignoring string to float conversion
// std::from_chars offers that with C++17, but isn't actually implemented yet for floats/doubles...
#include <private/qlocale_tools_p.h>

#include <cstdlib>
#include <limits>

using namespace KOSMIndoorMap;

// NOTE string to float conversion in here must be done ignoring the locale!
void LevelParser::parse(QByteArray &&level, OSM::Element e, const std::function<void(int, OSM::Element)> &callback)
{
    int rangeBegin = std::numeric_limits<int>::max();
    int numStartIdx = -1;

    for (int i = 0; i < level.size(); ++i) {
        auto &c = level[i];

        if (c == ',') { // fix decimal separator errors
            qCDebug(Log) << "syntax error in level tag:" << level << e.url();
            c = '.';
        }

        if (std::isdigit(static_cast<unsigned char>(c)) || c == '.') {
            if (numStartIdx < 0) {
                numStartIdx = i;
            }
            continue;
        }

        if (c == ';') {
            const int l = std::round(qstrtod(level.constData() + numStartIdx, nullptr, nullptr) * 10.0); // ### waiting for std::from_chars
            if (rangeBegin <= l) {
                for (int j = rangeBegin; j <= l; j += 10) {
                    callback(j, e);
                }
                rangeBegin = std::numeric_limits<int>::max();
            } else {
                callback(l, e);
            }
            numStartIdx = -1;
            continue;
        }

        if (c == QLatin1Char('-')) {
            if (numStartIdx < 0) {
                numStartIdx = i;
            } else {
                rangeBegin = std::round(qstrtod(level.constData() + numStartIdx, nullptr, nullptr) * 10.0); // ### waiting for std::from_chars
                numStartIdx = -1;
            }
        }
    }

    if (numStartIdx >= level.size() || numStartIdx < 0) {
        return;
    }
    const int l = std::round(qstrtod(level.constData() + numStartIdx, nullptr, nullptr) * 10.0); // ### waiting for std::from_chars
    if (rangeBegin <= l) {
        for (int j = rangeBegin; j <= l; j += 10) {
            callback(j, e);
        }
    } else {
        callback(l, e);
    }
}
