/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KOSMINDOORMAP_MAPCSSRESULT_P_H
#define KOSMINDOORMAP_MAPCSSRESULT_P_H

#include "kosmindoormap_export.h"

#include "mapcsstypes.h"

#include <qcompilerdetection.h>

#include <memory>
#include <vector>

class QByteArray;

namespace OSM {
class Tag;
class TagKey;
}

namespace KOSMIndoorMap {

class MapCSSDeclaration;
enum class MapCSSProperty;
class MapCSSRule;

class MapCSSResult;
class MapCSSResultLayerPrivate;

/** Result of MapCSS stylesheet evaluation for a single layer selector. */
class KOSMINDOORMAP_EXPORT MapCSSResultLayer
{
public:
    explicit MapCSSResultLayer();
    MapCSSResultLayer(const MapCSSResultLayer&) = delete;
    MapCSSResultLayer(MapCSSResultLayer&&) noexcept;
    MapCSSResultLayer& operator=(const MapCSSResultLayer&) = delete;
    MapCSSResultLayer& operator=(MapCSSResultLayer&&) noexcept;
    ~MapCSSResultLayer();

    void clear();

    /** Returns @c true if an area/polygon needs to be drawn. */
    [[nodiscard]] bool hasAreaProperties() const;
    /** Returns @c true if a way/line needs to be drawn. */
    [[nodiscard]] bool hasLineProperties() const;
    /** Returns @c true if a label needs to be drawn. */
    [[nodiscard]] bool hasLabelProperties() const;
    /** Returns @c true if a 3D extrusion is requested. */
    [[nodiscard]] bool hasExtrudeProperties() const;

    /** Returns the declaration for property @prop, or @c nullptr is this property isn't set. */
    [[nodiscard]] const MapCSSDeclaration* declaration(MapCSSProperty prop) const;
    /** The active declarations for the queried element. */
    [[nodiscard]] const std::vector<const MapCSSDeclaration*>& declarations() const;

    /** The layer selector for this result. */
    [[nodiscard]] LayerSelectorKey layerSelector() const;

    /** Tag lookup for tags overridden by the style sheet. */
    [[nodiscard]] QByteArray tagValue(OSM::TagKey key) const;

    /** Check whether this result layer has pseudo-class @p cls set. */
    [[nodiscard]] bool hasClass(ClassSelectorKey cls) const;

private:
    friend class MapCSSResult;
    friend class MapCSSRule;

    Q_DECL_HIDDEN void addDeclaration(const MapCSSDeclaration *decl);
    Q_DECL_HIDDEN void addClass(ClassSelectorKey cls);
    Q_DECL_HIDDEN void setLayerSelector(LayerSelectorKey layer);
    Q_DECL_HIDDEN void setTag(OSM::Tag &&tag);

    std::unique_ptr<MapCSSResultLayerPrivate> d;
};

class MapCSSBasicSelector;
class MapCSSResultPrivate;

/** Result of MapCSS stylesheet evaluation for all layer selectors.
 *  For performance reason it is highly recommended to reuse the same instance
 *  accross multiple style evaluations.
 */
class KOSMINDOORMAP_EXPORT MapCSSResult
{
public:
    explicit MapCSSResult();
    MapCSSResult(const MapCSSResultLayer&) = delete;
    MapCSSResult(MapCSSResult&&) noexcept;
    MapCSSResult& operator=(const MapCSSResult&) = delete;
    MapCSSResult& operator=(MapCSSResult&&) noexcept;
    ~MapCSSResult();

    /** Reset result state from a previous evaluation,
     *  while retaining previously allocated resource for reuse.
     */
    void clear();

    /** Results for all layer selectors. */
    [[nodiscard]] const std::vector<MapCSSResultLayer>& results() const;
    /** Access a specific result layer selector. */
    [[nodiscard]] const MapCSSResultLayer& operator[](LayerSelectorKey layer) const;
    [[nodiscard]] MapCSSResultLayer& operator[](LayerSelectorKey layer);

private:
    friend class MapCSSBasicSelector;
    /** Apply @p declarations for @p layer to the result. */
    Q_DECL_HIDDEN void applyDeclarations(LayerSelectorKey layer, const std::vector<std::unique_ptr<MapCSSDeclaration>> &declarations);

    std::unique_ptr<MapCSSResultPrivate> d;
};

}

#endif // KOSMINDOORMAP_MAPCSSRESULT_P_H
