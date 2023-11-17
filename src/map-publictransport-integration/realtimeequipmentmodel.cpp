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

constexpr inline const float EquipmentMatchDistance = 2.0; // meters

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
    Q_EMIT realtimeModelChanged();

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

static int matchCount(const std::vector<std::vector<int>> &matches, int idx)
{
    return std::accumulate(matches.begin(), matches.end(), 0, [idx](int count, const auto &indexes) {
        return count + std::count(indexes.begin(), indexes.end(), idx);
    });
}

static int findOtherMatch(const std::vector<std::vector<int>> &matches, int value, std::size_t current)
{
    for (std::size_t i = 0; i < matches.size(); ++i) {
        if (i == current) {
            continue;
        }
        if (std::find(matches[i].begin(), matches[i].end(), value) != matches[i].end()) {
            return i;
        }
    }
    Q_UNREACHABLE();
    return -1;
}

void RealtimeEquipmentModel::updateRealtimeState()
{
    m_pendingRealtimeUpdate = false;
    if (!m_realtimeModel) {
        return;
    }

    // clear previous data
    for (auto &eq : m_equipment) {
        if (!eq.syntheticElement) {
            continue;
        }
        eq.syntheticElement.removeTag(m_tagKeys.realtimeStatus);
    }

    // find candidates by distance
    std::vector<std::vector<int>> matches;
    matches.resize(m_equipment.size());
    for (auto i = 0; i < m_realtimeModel->rowCount(); ++i) {
        const auto idx = m_realtimeModel->index(i, 0);
        const auto loc = idx.data(KPublicTransport::LocationQueryModel::LocationRole).value<KPublicTransport::Location>();
        if (loc.type() != KPublicTransport::Location::Equipment) {
            continue;
        }

        const auto rtEq = loc.equipment();
        for (std::size_t j = 0; j < m_equipment.size(); ++j) {
            const auto &eq = m_equipment[j];
            if (!isSameEquipmentType(eq.type, rtEq.type())) {
                continue;
            }
            if (eq.distanceTo(m_data.dataSet(), loc.latitude(), loc.longitude()) < EquipmentMatchDistance) {
                matches[j].push_back(i);
            }
        }
    }

    // apply realtime status
    // we accept 3 different cases:
    // - a single 1:1 match
    // - a 1/2 or a 2/2 match for horizontally adjacent elements if there is a distance difference
    for (std::size_t i = 0; i < m_equipment.size(); ++i) {
        if (matches[i].size() == 1) {
            const auto mcount = matchCount(matches, matches[i][0]);
            if (mcount == 1) {
                const auto idx =  m_realtimeModel->index(matches[i][0], 0);
                const auto rtEq = idx.data(KPublicTransport::LocationQueryModel::LocationRole).value<KPublicTransport::Location>().equipment();
                updateEquipment(m_equipment[i], rtEq);
            }
            else if (mcount == 2) {
                const auto other = findOtherMatch(matches, matches[i][0], i);
                if (matches[other].size() == 2) {
                    const auto otherRow = matches[other][0] == matches[i][0] ? matches[other][1] : matches[other][0];
                    if (matchCount(matches, otherRow) == 1) {
                        resolveEquipmentPair(i, other, matches[other][0], matches[other][1]);
                    }
                }
            }
        }

        if (matches[i].size() == 2) {
            if (matchCount(matches, matches[i][0]) == 2 && matchCount(matches, matches[i][1]) == 2) {
                const auto it = std::find(std::next(matches.begin() + i), matches.end(), matches[i]);
                if (it != matches.end()) {
                    resolveEquipmentPair(i, std::distance(matches.begin(), it), matches[i][0], matches[i][1]);
                }
            }
        }
    }

    Q_EMIT update();
}
void RealtimeEquipmentModel::resolveEquipmentPair(int eqRow1, int eqRow2, int rtRow1, int rtRow2)
{
    // check if the equipment pair is horizontally adjacent
    if (m_equipment[eqRow1].levels != m_equipment[eqRow2].levels) {
        return;
    }

    // match best combination
    const auto rtIdx1 = m_realtimeModel->index(rtRow1, 0);
    const auto rtIdx2 = m_realtimeModel->index(rtRow2, 0);
    const auto rtEq1 = rtIdx1.data(KPublicTransport::LocationQueryModel::LocationRole).value<KPublicTransport::Location>();
    const auto rtEq2 = rtIdx2.data(KPublicTransport::LocationQueryModel::LocationRole).value<KPublicTransport::Location>();

    const auto d11 = m_equipment[eqRow1].distanceTo(m_data.dataSet(), rtEq1.latitude(), rtEq1.longitude());
    const auto d12 = m_equipment[eqRow1].distanceTo(m_data.dataSet(), rtEq2.latitude(), rtEq2.longitude());
    const auto d21 = m_equipment[eqRow2].distanceTo(m_data.dataSet(), rtEq1.latitude(), rtEq1.longitude());
    const auto d22 = m_equipment[eqRow2].distanceTo(m_data.dataSet(), rtEq2.latitude(), rtEq2.longitude());

    const auto swap1 = d11 >= d12;
    const auto swap2 = d21 < d22;

    if (swap1 != swap2) {
        return;
    }

    if (swap1) {
        if (d12 < EquipmentMatchDistance && d21 < EquipmentMatchDistance) {
            updateEquipment(m_equipment[eqRow1], rtEq2.equipment());
            updateEquipment(m_equipment[eqRow2], rtEq1.equipment());
        }
    } else {
        if (d11 < EquipmentMatchDistance && d22 < EquipmentMatchDistance) {
            updateEquipment(m_equipment[eqRow1], rtEq1.equipment());
            updateEquipment(m_equipment[eqRow2], rtEq2.equipment());
        }
    }
}

#include "moc_realtimeequipmentmodel.cpp"
