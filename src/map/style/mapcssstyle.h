/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KOSMINDOORMAP_MAPCSSSTYLE_H
#define KOSMINDOORMAP_MAPCSSSTYLE_H

#include "kosmindoormap_export.h"

#include <memory>

class QIODevice;

namespace OSM {
class DataSet;
}

namespace KOSMIndoorMap {

class MapCSSResult;
class MapCSSState;
class MapCSSStylePrivate;

class ClassSelectorKey;
class LayerSelectorKey;

/** A parsed MapCSS style sheet.
 *  @see MapCSSParser::parse for how to obtain a valid instance
 */
class KOSMINDOORMAP_EXPORT MapCSSStyle
{
public:
    /** Creates an invalid/empty style. */
    explicit MapCSSStyle();
    MapCSSStyle(const MapCSSStyle&) = delete;
    MapCSSStyle(MapCSSStyle&&) noexcept;
    ~MapCSSStyle();

    MapCSSStyle& operator=(const MapCSSStyle&) = delete;
    MapCSSStyle& operator=(MapCSSStyle&&) noexcept;

    /** Returns @c true if this is a default-constructed or otherwise empty/invalud style. */
    [[nodiscard]] bool isEmpty() const;

    /** Optimizes style sheet rules for application against @p dataSet.
     *  This does resolve tag keys and is therefore mandatory when changing the data set.
     */
    void compile(const OSM::DataSet &dataSet);

    /** Initializes the evaluation state.
     *  Call this on a MapCSSState instance for each element being evaluated.
     *  The state object can be reused for multiple elements to reduce allocations.
     *  The state object can also be reused for expression evaluations on the style
     *  sheet evaluation result.
     */
    void initializeState(MapCSSState &state) const;

    /** Evaluates the style sheet for a given state @p state (OSM element, view state, element state, etc).
     *  The result is not returned but added to @p result for reusing allocated memory
     *  between evaluations.
     *  @note @p state has to be initialized using MapCSSStyle initializeState() for this
     *  to produce correct results.
     */
    void evaluate(const MapCSSState &state, MapCSSResult &result) const;

    /** Evaluate canvas style rules. */
    void evaluateCanvas(const MapCSSState &state, MapCSSResult &result) const;

    /** Write this style as MapCSS to @p out.
     *  Mainly used for testing.
     */
    void write(QIODevice *out) const;

    /** Look up a class selector key for the given name, if it exists.
     *  If no such key exists in the style sheet, an null key is returned.
     *  Use this for checking if a class is set on an evaluation result.
     */
    [[nodiscard]] ClassSelectorKey classKey(const char *className) const;

    /** Look up a layer selector key for the given name, if it exists.
     *  If no such key exists in the style sheet, an null key is returned.
     *  Use this for accessing specific result layers.
     */
    [[nodiscard]] LayerSelectorKey layerKey(const char *layerName) const;

private:
    friend class MapCSSStylePrivate;
    std::unique_ptr<MapCSSStylePrivate> d;
};

}

#endif // KOSMINDOORMAP_MAPCSSSTYLE_H
