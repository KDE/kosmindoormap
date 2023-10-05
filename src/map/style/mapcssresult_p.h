/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KOSMINDOORMAP_MAPCSSRESULT_P_H
#define KOSMINDOORMAP_MAPCSSRESULT_P_H

#include "kosmindoormap_export.h"

#include "mapcssdeclaration_p.h"
#include "mapcsstypes.h"

#include <vector>

namespace OSM {
class Tag;
class TagKey;
}

namespace KOSMIndoorMap {

/** Result of MapCSS stylesheet evaluation for a single layer selector. */
class KOSMINDOORMAP_EXPORT MapCSSResultItem
{
public:
    explicit MapCSSResultItem();
    MapCSSResultItem(const MapCSSResultItem&) = delete;
    MapCSSResultItem(MapCSSResultItem&&) noexcept = default;
    MapCSSResultItem& operator=(const MapCSSResultItem&) = delete;
    MapCSSResultItem& operator=(MapCSSResultItem&&) noexcept = default;
    ~MapCSSResultItem();

    void clear();

    /** Returns @c true if an area/polygon needs to be drawn. */
    [[nodiscard]] bool hasAreaProperties() const;
    /** Returns @c true if a way/line needs to be drawn. */
    [[nodiscard]] bool hasLineProperties() const;
    /** Returns @c true if a label needs to be drawn. */
    [[nodiscard]] bool hasLabelProperties() const;

    /** Returns the declaration for property @prop, or @c nullptr is this property isn't set. */
    [[nodiscard]] const MapCSSDeclaration* declaration(MapCSSDeclaration::Property prop) const;
    /** The active declarations for the queried element. */
    [[nodiscard]] const std::vector<const MapCSSDeclaration*>& declarations() const;

    /** The layer selector for this result. */
    [[nodiscard]] LayerSelectorKey layerSelector() const;

    /** Tag lookup for tags overridden by the style sheet. */
    [[nodiscard]] QByteArray tagValue(OSM::TagKey key) const;

    /** @internal */
    void addDeclaration(const MapCSSDeclaration *decl);
    void addClass(ClassSelectorKey cls);
    [[nodiscard]] bool hasClass(ClassSelectorKey cls) const;
    void setLayerSelector(LayerSelectorKey layer);
    void setTag(OSM::Tag &&tag);

private:
    std::vector<const MapCSSDeclaration*> m_declarations;
    std::vector<ClassSelectorKey> m_classes;
    std::vector<OSM::Tag> m_tags;
    LayerSelectorKey m_layer;
    int m_flags = 0;
};

/** Result of MapCSS stylesheet evaluation for all layer selectors. */
class KOSMINDOORMAP_EXPORT MapCSSResult
{
public:
    explicit MapCSSResult();
    MapCSSResult(const MapCSSResultItem&) = delete;
    MapCSSResult(MapCSSResult&&) noexcept = default;
    MapCSSResult& operator=(const MapCSSResult&) = delete;
    MapCSSResult& operator=(MapCSSResult&&) noexcept = default;
    ~MapCSSResult();

    void clear();

    /** Results for all layer selectors. */
    const std::vector<MapCSSResultItem>& results() const;

    /** @internal */
    MapCSSResultItem& operator[](LayerSelectorKey layer);
    const MapCSSResultItem& operator[](LayerSelectorKey layer) const;
    /** Apply @p declarations for @p layer to the result. */
    void applyDeclarations(LayerSelectorKey layer, const std::vector<std::unique_ptr<MapCSSDeclaration>> &declarations);

private:
    std::vector<MapCSSResultItem> m_results;
    mutable std::vector<MapCSSResultItem> m_inactivePool; // for reuse of already allocated result items
};

}

#endif // KOSMINDOORMAP_MAPCSSRESULT_P_H
