/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.kosmindoormap
import org.kde.kosmindoorrouting

Kirigami.Page {
    title: map.floorLevels.hasName(map.view.floorLevel) && isNaN(parseInt(map.floorLevels.name(map.view.floorLevel))) ? map.floorLevels.name(map.view.floorLevel) : ("Floor " + map.floorLevels.name(map.view.floorLevel));
    property point coordinate
    property alias map: map
    property alias debug: infoModel.debug
    property alias mapHoverEnabled: hoverHandler.enabled
    property RoutingController routingController

    topPadding: 0
    bottomPadding: 0
    leftPadding: 0
    rightPadding: 0

    // prevent swipe to the right changing pages, we want to pan the map instead
    // TODO in theory we could make this conditional to having panned the map all the way to the right
    Kirigami.ColumnView.preventStealing: true

    actions: [
        Kirigami.Action {
            icon.name: "go-down-symbolic"
            enabled: map.floorLevels.hasFloorLevelBelow(map.view.floorLevel)
            onTriggered: map.view.floorLevel = map.floorLevels.floorLevelBelow(map.view.floorLevel)
            shortcut: "PgDown"
        },
        Kirigami.Action {
            icon.name: "go-up-symbolic"
            enabled: map.floorLevels.hasFloorLevelAbove(map.view.floorLevel)
            onTriggered: map.view.floorLevel = map.floorLevels.floorLevelAbove(map.view.floorLevel)
            shortcut: "PgUp"
        }
    ]

    OSMElementInformationModel {
        id: infoModel
        allowOnlineContent: true
        debug: true
    }

    IndoorMapInfoSheet {
        id: elementDetailsSheet
        model: infoModel
        regionCode: page.map.mapData.regionCode
        timeZone: page.map.mapData.timeZone
    }

    FloorLevelChangeModel {
        id: floorLevelChangeModel
        currentFloorLevel: map.view.floorLevel
        floorLevelModel: map.floorLevels
    }

    FloorLevelSelector {
        id: elevatorSheet
        model: floorLevelChangeModel
        onFloorLevelSelected: (level) => { map.view.floorLevel = level; }
    }

    IndoorMap {
        id: map
        anchors.fill: parent
        hoveredElement: map.elementAt(map.mapEventPointToScreen(hoverHandler.point))

        IndoorMapScale {
            map: map
            anchors.left: map.left
            anchors.bottom: map.bottom
            width: 0.3 * map.width
        }

        IndoorMapAttributionLabel {
            anchors.right: map.right
            anchors.bottom: map.bottom
        }

        QQC2.Menu {
            id: contextMenu
            property mapPointerEvent ev
            QQC2.MenuItem {
                text: i18n("Navigate from here")
                icon.name: "go-next"
                onTriggered: {
                    routingController.setStartPosition(contextMenu.ev.geoPosition.y, contextMenu.ev.geoPosition.x, map.view.floorLevel);
                    routingController.searchRoute();
                }
            }
            QQC2.MenuItem {
                text: i18n("Navigate to here")
                icon.name: "map-symbolic"
                onTriggered: {
                    routingController.setEndPosition(contextMenu.ev.geoPosition.y, contextMenu.ev.geoPosition.x, map.view.floorLevel);
                    routingController.searchRoute();
                }
            }
            QQC2.MenuItem {
                id: contextMenuInfoAction
                // enabled: !ev.element.isNull && (infoModel.name !== "" || infoModel.debug)
                text: i18n("Show information")
                icon.name: "documentinfo"
                onTriggered: elementDetailsSheet.open()
            }
        }

        function showContextMenu(ev) {
            infoModel.element = ev.element;
            contextMenuInfoAction.enabled = !ev.element.isNull && (infoModel.name !== "" || infoModel.debug);
            contextMenu.ev = ev;
            contextMenu.popup(map, ev.screenPosition);
        }

        onTapped: (ev) => {
            // left click on element: floor level change if applicable, otherwise info dialog
            // (button === is finger on touch screen)
            if (!ev.element.isNull && (ev.button === Qt.LeftButton || ev.button === 0)) {
                floorLevelChangeModel.element = ev.element;
                if (floorLevelChangeModel.hasSingleLevelChange) {
                    showPassiveNotification("Switched to floor " + floorLevelChangeModel.destinationLevelName, "short");
                    map.view.floorLevel = floorLevelChangeModel.destinationLevel;
                    return;
                } else if (floorLevelChangeModel.hasMultipleLevelChanges) {
                    elevatorSheet.open();
                    return;
                }

                infoModel.element = ev.element;
                if (infoModel.name != "" || infoModel.debug) {
                    elementDetailsSheet.open();
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

        HoverHandler {
            id: hoverHandler
            enabled: false
            acceptedDevices: PointerDevice.Mouse | PointerDevice.TouchPad
        }
    }

    onCoordinateChanged: map.mapLoader.loadForCoordinate(coordinate.y, coordinate.x);
}
