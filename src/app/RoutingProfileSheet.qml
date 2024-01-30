/*
    SPDX-FileCopyrightText: 2024 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.kosmindoorrouting

Kirigami.OverlaySheet {
    id: root

    property var routingProfile
    signal applyRoutingProfile()

    header: Kirigami.Heading {
        text: i18n("Configure Routing Profile")
    }

    Kirigami.FormLayout {
        Layout.preferredWidth: Kirigami.Units.gridUnit * 25
        Kirigami.Separator {
            Kirigami.FormData.isSection: true
            Kirigami.FormData.label: "Flags"
        }

        QQC2.CheckBox {
            id: stairFlag
            Kirigami.FormData.label: i18n("Use stairs")
            checked: routingProfile.flags & 2 // TODO export actual flags/enums!
        }
        QQC2.CheckBox {
            id: escalatorFlag
            Kirigami.FormData.label: i18n("Use escalators")
            checked: routingProfile.flags & 4
        }
        QQC2.CheckBox {
            id: elevatorFlag
            Kirigami.FormData.label: i18n("Use elevators")
            checked: routingProfile.flags & 8
        }

        Kirigami.Separator {
            Kirigami.FormData.isSection: true
            Kirigami.FormData.label: "Cost factors"
        }

        QQC2.TextField {
            id: stairCost
            Kirigami.FormData.label: i18n("Stairs")
            text: routingProfile.cost(1) // TODO see above
        }
        QQC2.TextField {
            id: elevatorCost
            Kirigami.FormData.label: i18n("Elevator")
            text: routingProfile.cost(2)
        }
        QQC2.TextField {
            id: escalatorCost
            Kirigami.FormData.label: i18n("Escalator")
            text: routingProfile.cost(3)
        }
        QQC2.TextField {
            id: movingWalkwayCost
            Kirigami.FormData.label: i18n("Moving Walkways")
            text: routingProfile.cost(4)
        }
        QQC2.TextField {
            id: tactilePavingCost
            Kirigami.FormData.label: i18n("Tactile Paving")
            text: routingProfile.cost(5)
        }
        QQC2.TextField {
            id: streetCrossingCost
            Kirigami.FormData.label: i18n("Street Crossing")
            text: routingProfile.cost(6)
        }
        QQC2.TextField {
            id: rampCost
            Kirigami.FormData.label: i18n("Ramp")
            text: routingProfile.cost(7)
        }
        QQC2.TextField {
            id: walkingCost
            Kirigami.FormData.label: i18n("Walking")
            text: routingProfile.cost(63)
        }
    }

    footer: QQC2.Button {
        text: i18n("Apply")
        icon.name: "dialog-ok-apply"
        onClicked: {
            root.routingProfile.flags = 1;
            if (stairFlag.checked)
                root.routingProfile.flags |= 2; // TODO
            if (escalatorFlag.checked)
                root.routingProfile.flags |= 4;
            if (elevatorFlag.checked)
                root.routingProfile.flags |= 8;

            root.routingProfile.setCost(1, stairCost.text); // TODO
            root.routingProfile.setCost(2, elevatorCost.text);
            root.routingProfile.setCost(3, escalatorCost.text);
            root.routingProfile.setCost(4, movingWalkwayCost.text);
            root.routingProfile.setCost(5, tactilePavingCost.text);
            root.routingProfile.setCost(6, streetCrossingCost.text);
            root.routingProfile.setCost(7, rampCost.text);
            root.routingProfile.setCost(63, walkingCost.text);

            root.close()
            root.applyRoutingProfile()
        }
    }
}
