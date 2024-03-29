/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KOSMINDOORMAP_REALTIMEEQUIPMENTMODEL_H
#define KOSMINDOORMAP_REALTIMEEQUIPMENTMODEL_H

#include <KOSMIndoorMap/EquipmentModel>

#include <qqmlregistration.h>

namespace KPublicTransport {
class Equipment;
}

class QAbstractItemModel;

namespace KOSMIndoorMap {

/** Elevator/escalator overlay source augmented with realtime status data where available. */
class RealtimeEquipmentModel : public EquipmentModel
{
    Q_OBJECT
    Q_PROPERTY(QObject* realtimeModel READ realtimeModel WRITE setRealtimeModel NOTIFY realtimeModelChanged)
    QML_ELEMENT

public:
    explicit RealtimeEquipmentModel(QObject *parent = nullptr);
    ~RealtimeEquipmentModel();

    QObject *realtimeModel() const;
    void setRealtimeModel(QObject *model);

Q_SIGNALS:
    void realtimeModelChanged();

private:
    void updateRealtimeState();
    void updateEquipment(Equipment &eq, const KPublicTransport::Equipment &rtEq) const;
    void resolveEquipmentPair(int eqRow1, int eqRow2, int rtRow1, int rtRow2);

    QPointer<QAbstractItemModel> m_realtimeModel;
    bool m_pendingRealtimeUpdate = false;
};

}

#endif // KOSMINDOORMAP_REALTIMEEQUIPMENTMODEL_H
