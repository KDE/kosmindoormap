/*
    SPDX-FileCopyrightText: ⓒ 2025 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.components as Components
import org.kde.kosmindoormap as KOSMIndoorMap

/** IndoorMap, but with a few batteries included:
 *  - scale
 *  - attribution label
 *  - floor level navigation
 *  - element information dialog
 *  - elevator/escalator model
 */
KOSMIndoorMap.IndoorMap {
    id: root

    /** Floor level change model used for user-driven floor level changes. */
    property KOSMIndoorMap.FloorLevelChangeModel floorLevelChangeModel: KOSMIndoorMap.FloorLevelChangeModel {
        currentFloorLevel: root.view.floorLevel
        floorLevelModel: root.floorLevels
    }

    /** Floor level selector popup, for elevators/staircases. */
    property KOSMIndoorMap.FloorLevelSelector floorLevelSelector: KOSMIndoorMap.FloorLevelSelector {
        model: root.floorLevelChangeModel
        onFloorLevelSelected: (level) => { root.view.floorLevel = level; }
    }

    /** Element detail information model. */
    property KOSMIndoorMap.OSMElementInformationModel elementInfoModel: KOSMIndoorMap.OSMElementInformationModel {}

    /** Element detail information dialog. */
    property KOSMIndoorMap.OSMElementInformationDialog elementInfoDialog: KOSMIndoorMap.OSMElementInformationDialog {
        model: root.elementInfoModel
        regionCode: root.mapData.regionCode
        timeZone: root.mapData.timeZone
    }

    /** Elevator/escalator model.
     *  Can be exchanged with KOSMIndoorMap.RealtimeEquipmentModel for elevator realtime data.
     */
    property KOSMIndoorMap.EquipmentModel equipmentModel: KOSMIndoorMap.EquipmentModel {
        mapData: root.mapData
    }

    /** Override to show a custom context menu on right click or long press. */
    function showContextMenu(ev: KOSMIndoorMap.mapPointerEvent) {
        root.elementInfoModel.element = ev.element;
        if (root.elementInfoModel.name != "" || root.elementInfoModel.debug) {
            root.elementInfoDialog.open();
        }
    }

    // internal
    KOSMIndoorMap.IndoorMapScale {
        map: root
        anchors.left: root.left
        anchors.top: root.top
        width: 0.3 * root.width
    }

    KOSMIndoorMap.IndoorMapAttributionLabel {
        anchors.right: root.right
        anchors.bottom: root.bottom
    }

    overlaySources: [ root.equipmentModel ]

    onTapped: (ev) => {
        // left click on element: floor level change if applicable, otherwise info dialog
        // (button === is finger on touch screen)
        if (!ev.element.isNull && (ev.button === Qt.LeftButton || ev.button === 0)) {
            floorLevelChangeModel.element = ev.element;
            if (floorLevelChangeModel.hasSingleLevelChange) {
                showPassiveNotification(i18nd("kosmindoormap", "Switched to floor %1", floorLevelChangeModel.destinationLevelName), "short");
                root.view.floorLevel = floorLevelChangeModel.destinationLevel;
                return;
            } else if (floorLevelChangeModel.hasMultipleLevelChanges) {
                root.floorLevelSelector.open();
                return;
            }

            root.elementInfoModel.element = ev.element;
            if (root.elementInfoModel.name != "" || root.elementInfoModel.debug) {
                root.elementInfoDialog.open();
            }
        }

        // right click: context menu, with info action if clicked on an element
        if (ev.button === Qt.RightButton) {
            showContextMenu(ev);
        }
    }

    onLongPressed: (ev) => {
        showContextMenu(ev);
    }

    Components.DoubleFloatingButton {
        anchors {
            right: root.right
            rightMargin: Kirigami.Units.largeSpacing
            bottom: root.bottom
            bottomMargin: Kirigami.Units.largeSpacing + Kirigami.Units.smallSpacing // to not hide the copyright information
        }

        leadingAction: Kirigami.Action {
            icon.name: "go-down-symbolic"
            text: i18ndc("kosmindoormap", "@action:intoolbar Go down one floor", "Floor down")
            enabled: root.floorLevels.hasFloorLevelBelow(root.view.floorLevel)
            onTriggered: root.view.floorLevel = root.floorLevels.floorLevelBelow(root.view.floorLevel)
            visible: root.floorLevels.hasFloorLevels
            tooltip: text
        }

        trailingAction: Kirigami.Action {
            icon.name: "go-up-symbolic"
            text: i18ndc("kosmindoormap", "@action:intoolbar Go up one floor", "Floor up")
            enabled: root.floorLevels.hasFloorLevelAbove(root.view.floorLevel)
            onTriggered: root.view.floorLevel = root.floorLevels.floorLevelAbove(root.view.floorLevel)
            visible: root.floorLevels.hasFloorLevels
            tooltip: text
        }
    }
}
