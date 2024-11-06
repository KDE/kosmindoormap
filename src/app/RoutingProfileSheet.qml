/*
    SPDX-FileCopyrightText: 2024 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.kosmindoorrouting

Kirigami.Dialog {
    id: root

    property routingProfile routingProfile
    signal applyRoutingProfile()

    title: i18n("Configure Routing Profile")

    contentItem: Kirigami.FormLayout {
        Layout.preferredWidth: Kirigami.Units.gridUnit * 25
        Kirigami.Separator {
            Kirigami.FormData.isSection: true
            Kirigami.FormData.label: "Flags"
        }

        QQC2.CheckBox {
            id: stairFlag
            Kirigami.FormData.label: i18n("Use stairs")
        }
        QQC2.CheckBox {
            id: escalatorFlag
            Kirigami.FormData.label: i18n("Use escalators")
        }
        QQC2.CheckBox {
            id: elevatorFlag
            Kirigami.FormData.label: i18n("Use elevators")
        }

        Kirigami.Separator {
            Kirigami.FormData.isSection: true
            Kirigami.FormData.label: "Cost factors"
        }

        QQC2.TextField {
            id: stairCost
            Kirigami.FormData.label: i18n("Stairs")
        }
        QQC2.TextField {
            id: elevatorCost
            Kirigami.FormData.label: i18n("Elevator")
        }
        QQC2.TextField {
            id: escalatorCost
            Kirigami.FormData.label: i18n("Escalator")
        }
        QQC2.TextField {
            id: movingWalkwayCost
            Kirigami.FormData.label: i18n("Moving Walkways")
        }
        QQC2.TextField {
            id: tactilePavingCost
            Kirigami.FormData.label: i18n("Tactile Paving")
        }
        QQC2.TextField {
            id: streetCrossingCost
            Kirigami.FormData.label: i18n("Street Crossing")
        }
        QQC2.TextField {
            id: rampCost
            Kirigami.FormData.label: i18n("Ramp")
        }
        QQC2.TextField {
            id: roomCost
            Kirigami.FormData.label: i18n("Room")
        }
        QQC2.TextField {
            id: walkingCost
            Kirigami.FormData.label: i18n("Walking")
        }
    }

    onOpened: {
        stairFlag.checked = root.routingProfile.flags & AreaFlag.Stairs;
        escalatorFlag.checked = root.routingProfile.flags & AreaFlag.Escalator;
        elevatorFlag.checked = root.routingProfile.flags & AreaFlag.Elevator;

        stairCost.text = root.routingProfile.cost(AreaType.Stairs);
        elevatorCost.text = root.routingProfile.cost(AreaType.Elevator);
        escalatorCost.text = root.routingProfile.cost(AreaType.Escalator);
        movingWalkwayCost.text = root.routingProfile.cost(AreaType.MovingWalkway);
        tactilePavingCost.text = root.routingProfile.cost(AreaType.TactilePaving);
        streetCrossingCost.text = root.routingProfile.cost(AreaType.StreetCrossing);
        rampCost.text = root.routingProfile.cost(AreaType.Ramp);
        roomCost.text = root.routingProfile.cost(AreaType.Room);
        walkingCost.text = root.routingProfile.cost(AreaType.Walkable);
    }

    customFooterActions: [
        Kirigami.Action {
            text: i18n("Apply")
            icon.name: "dialog-ok-apply"
            onTriggered: {
                let flags = AreaFlag.Walkable;
                if (stairFlag.checked)
                    flags |= AreaFlag.Stairs;
                if (escalatorFlag.checked)
                    flags |= AreaFlag.Escalator;
                if (elevatorFlag.checked)
                    flags |= AreaFlag.Elevator;
                root.routingProfile.flags = flags;

                root.routingProfile.setCost(AreaType.Stairs, stairCost.text);
                root.routingProfile.setCost(AreaType.Elevator, elevatorCost.text);
                root.routingProfile.setCost(AreaType.Escalator, escalatorCost.text);
                root.routingProfile.setCost(AreaType.MovingWalkway, movingWalkwayCost.text);
                root.routingProfile.setCost(AreaType.TactilePaving, tactilePavingCost.text);
                root.routingProfile.setCost(AreaType.StreetCrossing, streetCrossingCost.text);
                root.routingProfile.setCost(AreaType.Ramp, rampCost.text);
                root.routingProfile.setCost(AreaType.Room, roomCost.text);
                root.routingProfile.setCost(AreaType.Walkable, walkingCost.text);

                root.close()
                root.applyRoutingProfile()
            }
        }
    ]
}
