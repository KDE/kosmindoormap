/*
    SPDX-FileCopyrightText: 2024 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KOSMINDOORMAP_MAPCSSVALUE_P_H
#define KOSMINDOORMAP_MAPCSSVALUE_P_H

#include "kosmindoormap_export.h"

#include <QVariant>

class QIODevice;

namespace KOSMIndoorMap {

/** (Intermediate) Result of an eval() expression.
 *  @see https://wiki.openstreetmap.org/wiki/MapCSS/0.2/eval#Data_types
 */
class KOSMINDOORMAP_EXPORT MapCSSValue {
public:
    MapCSSValue();
    MapCSSValue(const QByteArray &str); // implicit
    MapCSSValue(double num); // implicit
    MapCSSValue(bool b); // implicit
    ~MapCSSValue();

    /** MapCSS' equivalent to a null type. */
    [[nodiscard]] bool isNone() const;
    [[nodiscard]] QByteArray asString() const;
    [[nodiscard]] double asNumber() const;
    [[nodiscard]] bool asBoolean() const;

    [[nodiscard]] bool compareEqual(const MapCSSValue &other) const;

    /// @internal
    void write(QIODevice *out) const;

private:
    QVariant m_value;
};

}

#endif
