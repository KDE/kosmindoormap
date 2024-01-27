/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KOSMINDOORMAP_OVERLAYSOURCE_H
#define KOSMINDOORMAP_OVERLAYSOURCE_H

#include "kosmindoormap_export.h"

#include <KOSM/Element>

#include <QObject>
#include <QPointer>

#include <functional>

class QAbstractItemModel;

namespace KOSMIndoorMap {

class AbstractOverlaySourcePrivate;

/** A source for overlay elements, drawn on top of the static map data. */
class KOSMINDOORMAP_EXPORT AbstractOverlaySource : public QObject
{
    Q_OBJECT
public:
    virtual ~AbstractOverlaySource();

    /** Iteration interface with floor level filtering. */
    virtual void forEach(int floorLevel, const std::function<void(OSM::Element, int)> &func) const = 0;

    /** Adds hidden elements to @param elems. */
    virtual void hiddenElements(std::vector<OSM::Element> &elems) const;

    /** Indicates the being of a scene graph update.
     *  @see SceneGraph
     */
    virtual void beginSwap();
    /** Indicates the end of a scene graph update.
     *  At this point dynamically created and no longer needed OSM elements can safely be deleted.
     */
    virtual void endSwap();

    // HACK this still needs a proper solution for dynamic geometry
    /** Nodes for newly created geometry. */
    [[nodiscard]] virtual const std::vector<OSM::Node>* transientNodes() const;

Q_SIGNALS:
    /** Trigger map re-rendering when the source changes. */
    void update();

    /** Trigger style re-compilation.
     *  This is needed for example when the source added new tag keys that the map data
     *  didn't previously contain (and thus would be optimized out of the style).
     */
    void reset();

protected:
    explicit AbstractOverlaySource(QObject *parent);
    explicit AbstractOverlaySource(AbstractOverlaySourcePrivate *dd, QObject *parent);
    std::unique_ptr<AbstractOverlaySourcePrivate> d_ptr;
    Q_DECLARE_PRIVATE(AbstractOverlaySource)
};

class ModelOverlaySourcePrivate;

/** A source for overlay elements, based on a QAbstractItemModel as input. */
class KOSMINDOORMAP_EXPORT ModelOverlaySource : public AbstractOverlaySource
{
    Q_OBJECT
public:
    explicit ModelOverlaySource(QAbstractItemModel *model, QObject *parent = nullptr);
    ~ModelOverlaySource();

    /** Iteration interface with floor level filtering. */
    void forEach(int floorLevel, const std::function<void(OSM::Element, int)> &func) const override;

    /** Adds hidden elements to @param elems. */
    void hiddenElements(std::vector<OSM::Element> &elems) const override;

private:
    Q_DECLARE_PRIVATE(ModelOverlaySource)
};

}

#endif // KOSMINDOORMAP_OVERLAYSOURCE_H
