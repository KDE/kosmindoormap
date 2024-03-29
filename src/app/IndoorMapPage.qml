/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.kosmindoormap

Kirigami.Page {
    title: map.floorLevels.hasName(map.view.floorLevel) ? map.floorLevels.name(map.view.floorLevel) : ("Floor " + map.floorLevels.name(map.view.floorLevel));
    property point coordinate
    property alias map: map
    property alias debug: infoModel.debug
    property alias mapHoverEnabled: hoverHandler.enabled

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
        },
        Kirigami.Action {
            icon.name: "go-up-symbolic"
            enabled: map.floorLevels.hasFloorLevelAbove(map.view.floorLevel)
            onTriggered: map.view.floorLevel = map.floorLevels.floorLevelAbove(map.view.floorLevel)
        }
    ]

    OSMElementInformationModel {
        id: infoModel
        debug: true
    }

    IndoorMapInfoSheet {
        id: elementDetailsSheet
        model: infoModel
        mapData: page.map.mapData
    }

    FloorLevelChangeModel {
        id: floorLevelChangeModel
        currentFloorLevel: map.view.floorLevel
        floorLevelModel: map.floorLevels
    }

    Kirigami.OverlaySheet {
        id: elevatorSheet
        header: Kirigami.Heading {
            text: floorLevelChangeModel.title
        }
        ListView {
            model: floorLevelChangeModel
            Layout.preferredWidth: Kirigami.Units.gridUnit * 10
            delegate: QQC2.ItemDelegate {
                highlighted: false
                width: ListView.view.width
                contentItem: Kirigami.TitleSubtitle {
                    title: model.display
                    font.bold: model.isCurrentFloor
                }
                onClicked: {
                    elevatorSheet.close();
                    map.view.floorLevel = model.floorLevel;
                }
            }
        }
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

        onElementPicked: {
            floorLevelChangeModel.element = element;
            if (floorLevelChangeModel.hasSingleLevelChange) {
                showPassiveNotification("Switched to floor " + floorLevelChangeModel.destinationLevelName, "short");
                map.view.floorLevel = floorLevelChangeModel.destinationLevel;
                return;
            } else if (floorLevelChangeModel.hasMultipleLevelChanges) {
                elevatorSheet.open();
                return;
            }

            infoModel.element = element;
            if (infoModel.name != "" || infoModel.debug) {
                elementDetailsSheet.open();
            }
        }
        onElementLongPressed: {
            // this provides info model access for elements with other interactions
            infoModel.element = element;
            if (infoModel.name != "" || infoModel.debug) {
                elementDetailsSheet.open();
            }
        }

        HoverHandler {
            id: hoverHandler
            enabled: false
            acceptedDevices: PointerDevice.Mouse | PointerDevice.TouchPad
        }
    }

    onCoordinateChanged: map.mapLoader.loadForCoordinate(coordinate.y, coordinate.x);
}
