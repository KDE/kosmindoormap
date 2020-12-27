/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "realtimeequipmentmodel.h"

#include <KPublicTransport/Equipment>
#include <KPublicTransport/Location>
#include <KPublicTransport/LocationQueryModel>

#include <QAbstractItemModel>

using namespace KOSMIndoorMap;

RealtimeEquipmentModel::RealtimeEquipmentModel(QObject *parent)
    : EquipmentModel(parent)
{
}

RealtimeEquipmentModel::~RealtimeEquipmentModel() = default;

QObject* RealtimeEquipmentModel::realtimeModel() const
{
    return m_realtimeModel;
}

void RealtimeEquipmentModel::setRealtimeModel(QObject *model)
{
    if (m_realtimeModel == model) {
        return;
    }

    m_realtimeModel = qobject_cast<QAbstractItemModel*>(model);
    emit realtimeModelChanged();

    if (m_realtimeModel) {
        connect(m_realtimeModel, &QAbstractItemModel::modelReset, this, &RealtimeEquipmentModel::updateRealtimeState);
        connect(m_realtimeModel, &QAbstractItemModel::rowsInserted, this, [this](const auto &parent, auto first, auto last) {
            if (parent.isValid() || m_pendingRealtimeUpdate) {
                return;
            }
            for (auto i = first; i <= last; ++i) {
                const auto idx = m_realtimeModel->index(i, 0);
                const auto loc = idx.data(KPublicTransport::LocationQueryModel::LocationRole).template value<KPublicTransport::Location>();
                if (loc.type() == KPublicTransport::Location::Equipment) {
                    m_pendingRealtimeUpdate = true;
                    QMetaObject::invokeMethod(this, &RealtimeEquipmentModel::updateRealtimeState, Qt::QueuedConnection);
                    return;
                }
            }
        });
        connect(m_realtimeModel, &QAbstractItemModel::rowsRemoved, this, &RealtimeEquipmentModel::updateRealtimeState);
        connect(m_realtimeModel, &QAbstractItemModel::dataChanged, this, [this](const auto &fromIdx, const auto &toIdx) {
            if (m_pendingRealtimeUpdate) {
                return;
            }
            for (auto i = fromIdx.row(); i <= toIdx.row(); ++i) {
                const auto idx = m_realtimeModel->index(i, 0);
                const auto loc = idx.data(KPublicTransport::LocationQueryModel::LocationRole).template value<KPublicTransport::Location>();
                if (loc.type() == KPublicTransport::Location::Equipment) {
                    m_pendingRealtimeUpdate = true;
                    QMetaObject::invokeMethod(this, &RealtimeEquipmentModel::updateRealtimeState, Qt::QueuedConnection);
                    return;
                }
            }
        });

        if (m_realtimeModel->rowCount() > 0) {
            updateRealtimeState();
        }
    }
}

static bool isSameEquipmentType(Equipment::Type lhs, KPublicTransport::Equipment::Type rhs)
{
    return (lhs == Equipment::Elevator && rhs == KPublicTransport::Equipment::Elevator)
        || (lhs == Equipment::Escalator && rhs == KPublicTransport::Equipment::Escalator);
}

void RealtimeEquipmentModel::updateEquipment(Equipment &eq, const KPublicTransport::Equipment &rtEq) const
{
    createSyntheticElement(eq);
    eq.syntheticElement.setTagValue(m_tagKeys.realtimeStatus, rtEq.disruptionEffect() == KPublicTransport::Disruption::NoService ? "0" : "1");
}

void RealtimeEquipmentModel::updateRealtimeState()
{
    m_pendingRealtimeUpdate = false;
    if (!m_realtimeModel) {
        return;
    }

    for (auto i = 0; i < m_realtimeModel->rowCount(); ++i) {
        const auto idx = m_realtimeModel->index(i, 0);
        const auto loc = idx.data(KPublicTransport::LocationQueryModel::LocationRole).value<KPublicTransport::Location>();
        if (loc.type() != KPublicTransport::Location::Equipment) {
            continue;
        }

        const auto rtEq = loc.equipment();
        qDebug() << "trying to match equipment" << loc.name() << rtEq.type() << rtEq.disruptionEffect();
        auto eqIdx = std::numeric_limits<std::size_t>::max();
        for (std::size_t j = 0; j < m_equipment.size(); ++j) {
            const auto &eq = m_equipment[j];
            if (!isSameEquipmentType(eq.type, rtEq.type())) {
                continue;
            }
            if (eq.distanceTo(m_data.dataSet(), loc.latitude(), loc.longitude()) < 2.0) {
                if (eqIdx < m_equipment.size()) {
                    qDebug() << "  multiple hits for equipment!" << loc.name();
                    eqIdx = std::numeric_limits<std::size_t>::max();
                    break;
                } else {
                    eqIdx = j;
                }
            }
        }

        if (eqIdx < m_equipment.size()) {
            qDebug() << "  found equipment!" << loc.name();
            auto &eq = m_equipment[eqIdx];
            updateEquipment(eq, rtEq);
        }
    }

    emit update();
}
