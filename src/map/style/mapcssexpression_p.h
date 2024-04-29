/*
    SPDX-FileCopyrightText: 2024 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KOSMINDOORMAP_MAPCSSEXPRESSION_P_H
#define KOSMINDOORMAP_MAPCSSEXPRESSION_P_H

#include "kosmindoormap_export.h"

#include <memory>

class QIODevice;

namespace OSM { class DataSet; }

namespace KOSMIndoorMap {

class MapCSSResultLayer;
class MapCSSState;
class MapCSSTerm;
class MapCSSValue;

/** A MapCSS eval() expression.
 *  @internal only exported for testing
 */
class KOSMINDOORMAP_EXPORT MapCSSExpression {
public:
    MapCSSExpression();
    explicit MapCSSExpression(MapCSSTerm *term);
    MapCSSExpression(MapCSSExpression&&) noexcept;
    ~MapCSSExpression();

    MapCSSExpression& operator=(MapCSSExpression&&) noexcept;

    /** Checks whether this is a valid expression. */
    [[nodiscard]] bool isValid() const;

    /** Optimize expression for use on @p dataSet. */
    void compile(const OSM::DataSet &dataSet);

    /** Evaluate the expression given the context of
     *  - the currently evaluated element and view state
     *  - the style evaluation result
     */
    [[nodiscard]] MapCSSValue evaluate(const MapCSSState &state, const MapCSSResultLayer &result) const;

    /** Parse a stand-alone eval() expression. */
    // TODO error reporting
    [[nodiscard]] static MapCSSExpression fromString(const char *str);

    void write(QIODevice *out) const;
private:
    std::unique_ptr<MapCSSTerm> m_term;
};

}

#endif
