/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KOSMINDOORMAP_MAPCSSRESULT_P_H
#define KOSMINDOORMAP_MAPCSSRESULT_P_H

#include "mapcssdeclaration_p.h"
#include "mapcsstypes.h"

#include <vector>

namespace KOSMIndoorMap {

/** Result of MapCSS stylesheet evaluation for a single layer selector. */
class MapCSSResultItem
{
public:
    explicit MapCSSResultItem();
    ~MapCSSResultItem();

    void clear();

    /** Returns @c true if an area/polygon needs to be drawn. */
    bool hasAreaProperties() const;
    /** Returns @c true if a way/line needs to be drawn. */
    bool hasLineProperties() const;
    /** Returns @c true if a label needs to be drawn. */
    bool hasLabelProperties() const;

    /** Returns the declaration for property @prop, or @c nullptr is this property isn't set. */
    const MapCSSDeclaration* declaration(MapCSSDeclaration::Property prop) const;
    /** The active declarations for the queried element. */
    const std::vector<const MapCSSDeclaration*>& declarations() const;

    /** The layer selector for this result. */
    LayerSelectorKey layerSelector() const;

    /** @internal */
    void addDeclaration(const MapCSSDeclaration *decl);
    void addClass(ClassSelectorKey cls);
    bool hasClass(ClassSelectorKey cls) const;
    void setLayerSelector(LayerSelectorKey layer);

private:
    std::vector<const MapCSSDeclaration*> m_declarations;
    std::vector<ClassSelectorKey> m_classes;
    LayerSelectorKey m_layer;
    int m_flags = 0;
};

/** Result of MapCSS stylesheet evaluation for all layer selectors. */
class MapCSSResult
{
public:
    explicit MapCSSResult();
    ~MapCSSResult();

    void clear();

    /** Results for all layer selectors. */
    const std::vector<MapCSSResultItem>& results() const;

    /** @internal */
    MapCSSResultItem& operator[](LayerSelectorKey layer);
    const MapCSSResultItem& operator[](LayerSelectorKey layer) const;

private:
    std::vector<MapCSSResultItem> m_results;
    mutable std::vector<MapCSSResultItem> m_inactivePool; // for reuse of already allocated result items
};

}

#endif // KOSMINDOORMAP_MAPCSSRESULT_P_H
