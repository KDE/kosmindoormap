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

    property var routingProfile
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
            checked: routingProfile.flags & AreaFlag.Stairs
        }
        QQC2.CheckBox {
            id: escalatorFlag
            Kirigami.FormData.label: i18n("Use escalators")
            checked: routingProfile.flags & AreaFlag.Escalator
        }
        QQC2.CheckBox {
            id: elevatorFlag
            Kirigami.FormData.label: i18n("Use elevators")
            checked: routingProfile.flags & AreaFlag.Elevator
        }

        Kirigami.Separator {
            Kirigami.FormData.isSection: true
            Kirigami.FormData.label: "Cost factors"
        }

        QQC2.TextField {
            id: stairCost
            Kirigami.FormData.label: i18n("Stairs")
            text: routingProfile.cost(AreaType.Stairs)
        }
        QQC2.TextField {
            id: elevatorCost
            Kirigami.FormData.label: i18n("Elevator")
            text: routingProfile.cost(AreaType.Elevator)
        }
        QQC2.TextField {
            id: escalatorCost
            Kirigami.FormData.label: i18n("Escalator")
            text: routingProfile.cost(AreaType.Escalator)
        }
        QQC2.TextField {
            id: movingWalkwayCost
            Kirigami.FormData.label: i18n("Moving Walkways")
            text: routingProfile.cost(AreaType.MovingWalkway)
        }
        QQC2.TextField {
            id: tactilePavingCost
            Kirigami.FormData.label: i18n("Tactile Paving")
            text: routingProfile.cost(AreaType.TactilePaving)
        }
        QQC2.TextField {
            id: streetCrossingCost
            Kirigami.FormData.label: i18n("Street Crossing")
            text: routingProfile.cost(AreaType.StreetCrossing)
        }
        QQC2.TextField {
            id: rampCost
            Kirigami.FormData.label: i18n("Ramp")
            text: routingProfile.cost(AreaType.Ramp)
        }
        QQC2.TextField {
            id: roomCost
            Kirigami.FormData.label: i18n("Room")
            text: routingProfile.cost(AreaType.Room)
        }
        QQC2.TextField {
            id: walkingCost
            Kirigami.FormData.label: i18n("Walking")
            text: routingProfile.cost(AreaType.Walkable)
        }
    }

    customFooterActions: [
        Kirigami.Action {
            text: i18n("Apply")
            icon.name: "dialog-ok-apply"
            onTriggered: {
                root.routingProfile.flags = AreaFlag.Walkable;
                if (stairFlag.checked)
                    root.routingProfile.flags |= AreaFlag.Stairs;
                if (escalatorFlag.checked)
                    root.routingProfile.flags |= AreaFlag.Escalator;
                if (elevatorFlag.checked)
                    root.routingProfile.flags |= AreaFlag.Elevator;

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
