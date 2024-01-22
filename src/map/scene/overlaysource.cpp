/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "overlaysource.h"

#include <QAbstractItemModel>
#include <QDebug>

using namespace KOSMIndoorMap;

namespace KOSMIndoorMap
{
class AbstractOverlaySourcePrivate {
public:
    virtual ~AbstractOverlaySourcePrivate() = default;
};

class ModelOverlaySourcePrivate : public AbstractOverlaySourcePrivate {
public:
    ~ModelOverlaySourcePrivate() override = default;
    void recursiveForEach(const QModelIndex &rootIdx, int floorLevel, const std::function<void (OSM::Element, int)> &func) const;

    QPointer<QAbstractItemModel> m_model;
    int m_elementRole = -1;
    int m_floorRole = -1;
    int m_hiddenElementRole = -1;
};

}

AbstractOverlaySource::AbstractOverlaySource(QObject *parent)
    : AbstractOverlaySource(new AbstractOverlaySourcePrivate, parent)
{
}

AbstractOverlaySource::AbstractOverlaySource(AbstractOverlaySourcePrivate *dd, QObject *parent)
    : QObject(parent)
    , d_ptr(dd)
{
}

AbstractOverlaySource::~AbstractOverlaySource() = default;

void AbstractOverlaySource::hiddenElements([[maybe_unused]] std::vector<OSM::Element> &elems) const
{
}


ModelOverlaySource::ModelOverlaySource(QAbstractItemModel *model, QObject *parent)
    : AbstractOverlaySource(new ModelOverlaySourcePrivate, parent)
{
    Q_D(ModelOverlaySource);
    const auto roles = model->roleNames();
    for (auto it = roles.begin(); it != roles.end(); ++it) {
        if (it.value() == "osmElement") {
            d->m_elementRole = it.key();
        } else if (it.value() == "level") {
            d->m_floorRole = it.key();
        } else if (it.value() == "hiddenElement") {
            d->m_hiddenElementRole = it.key();
        }
    }
    if (d->m_elementRole < 0 || d->m_floorRole < 0) {
        qWarning() << model << " - model does not provide the required roles!";
        return;
    }
    d->m_model = model;

    connect(model, &QAbstractItemModel::modelReset, this, &ModelOverlaySource::update);
    connect(model, &QAbstractItemModel::rowsInserted, this, &ModelOverlaySource::update);
    connect(model, &QAbstractItemModel::rowsRemoved, this, &ModelOverlaySource::update);
    connect(model, &QAbstractItemModel::dataChanged, this, &ModelOverlaySource::update);

    connect(model, &QAbstractItemModel::modelReset, this, &ModelOverlaySource::reset);
}

ModelOverlaySource::~ModelOverlaySource() = default;

void ModelOverlaySource::forEach(int floorLevel, const std::function<void (OSM::Element, int)> &func) const
{
    Q_D(const ModelOverlaySource);
    if (!d->m_model) {
        return;
    }

    d->recursiveForEach({}, floorLevel, func);
}

void ModelOverlaySourcePrivate::recursiveForEach(const QModelIndex &rootIdx, int floorLevel, const std::function<void (OSM::Element, int)> &func) const
{
    const auto rows = m_model->rowCount(rootIdx);
    for (int i = 0; i < rows; ++i) {
        const auto idx = m_model->index(i, 0, rootIdx);
        const auto floor = idx.data(m_floorRole).toInt();
        if (floor != floorLevel) {
            continue;
        }

        recursiveForEach(idx, floorLevel, func);

        const auto elem = idx.data(m_elementRole).value<OSM::Element>();
        func(elem, floor);
    }
}

void ModelOverlaySource::hiddenElements(std::vector<OSM::Element> &elems) const
{
    Q_D(const ModelOverlaySource);
    if (!d->m_model || d->m_hiddenElementRole < 0) {
        return;
    }

    const auto rows = d->m_model->rowCount();
    for (int i = 0; i < rows; ++i) {
        const auto idx = d->m_model->index(i, 0);
        const auto elem = idx.data(d->m_hiddenElementRole).value<OSM::Element>();
        if (elem.type() != OSM::Type::Null) {
            elems.push_back(elem);
        }
    }
}

#include "moc_overlaysource.cpp"
