/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KOSMINDOORMAP_MAPCSSSELECTOR_P_H
#define KOSMINDOORMAP_MAPCSSSELECTOR_P_H

#include "mapcsstypes.h"
#include "mapcssobjecttype_p.h"

#include <osm/datatypes.h>

#include <functional>
#include <memory>
#include <vector>

class QIODevice;

namespace KOSMIndoorMap {

class MapCSSCondition;
class MapCSSConditionHolder;
class MapCSSDeclaration;
class MapCSSResult;
class MapCSSState;

/** Base class for a style selector. */
class MapCSSSelector
{
public:
    virtual ~MapCSSSelector();

    /** Resolve tag keys. */
    virtual void compile(const OSM::DataSet &dataSet) = 0;
    /** Returns @c true if this selector matches the evaluation state. */
    virtual bool matches(const MapCSSState &state, MapCSSResult &result, const std::vector<std::unique_ptr<MapCSSDeclaration>> &declarations) const = 0;
    /** Selector matches the canvas element. */
    virtual bool matchesCanvas(const MapCSSState &state) const = 0;
    /** The layer selector of this style selector (invalid for union selectors). */
    virtual LayerSelectorKey layerSelector() const = 0;

    virtual void write(QIODevice *out) const = 0;

protected:
    explicit MapCSSSelector();
};

/** Basic selector, ie one that only contains tests but no sub-selectors. */
class MapCSSBasicSelector : public MapCSSSelector
{
public:
    explicit MapCSSBasicSelector();
    ~MapCSSBasicSelector();

    void compile(const OSM::DataSet &dataSet) override;
    [[nodiscard]] bool matches(const MapCSSState &state, MapCSSResult &result, const std::vector<std::unique_ptr<MapCSSDeclaration>> &declarations) const override;
    [[nodiscard]] bool matchesCanvas(const MapCSSState &state) const override;
    [[nodiscard]] LayerSelectorKey layerSelector() const override;
    void write(QIODevice* out) const override;

    /** @internal only to be used by the parser */
    void setObjectType(const char *str, std::size_t len);
    void setZoomRange(int low, int high);
    void setConditions(MapCSSConditionHolder *conds);
    void setClass(ClassSelectorKey key);
    void setPseudoClass(const char *str, std::size_t len);
    void setLayer(LayerSelectorKey key);

private:
    MapCSSObjectType m_objectType = MapCSSObjectType::Any;
    std::vector<std::unique_ptr<MapCSSCondition>> conditions;
    ClassSelectorKey m_class;
    LayerSelectorKey m_layer;
    int m_zoomLow = 0;
    int m_zoomHigh = 0;
};

/** Selector chain. */
class MapCSSChainedSelector : public MapCSSSelector
{
public:
    void compile(const OSM::DataSet &dataSet) override;
    bool matches(const MapCSSState &state, MapCSSResult &result, const std::vector<std::unique_ptr<MapCSSDeclaration>> &declarations) const override;
    bool matchesCanvas(const MapCSSState &state) const override;
    LayerSelectorKey layerSelector() const override;
    void write(QIODevice* out) const override;
    std::vector<std::unique_ptr<MapCSSBasicSelector>> selectors;
};

/** Union selector set. */
class MapCSSUnionSelector : public MapCSSSelector
{
public:
    explicit MapCSSUnionSelector();
    ~MapCSSUnionSelector();

    void compile(const OSM::DataSet &dataSet) override;
    bool matches(const MapCSSState &state, MapCSSResult &result, const std::vector<std::unique_ptr<MapCSSDeclaration>> &declarations) const override;
    bool matchesCanvas(const MapCSSState &state) const override;
    LayerSelectorKey layerSelector() const override;
    void write(QIODevice* out) const override;

    /** @internal */
    void addSelector(std::unique_ptr<MapCSSSelector> &&selector);

private:
    struct SelectorMap {
        LayerSelectorKey layer;
        std::vector<std::unique_ptr<MapCSSSelector>> selectors;
    };
    std::vector<SelectorMap> m_selectors;
};

}

#endif // KOSMINDOORMAP_MAPCSSSELECTOR_P_H
