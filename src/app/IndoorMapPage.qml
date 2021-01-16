/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.12
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.1 as QQC2
import org.kde.kirigami 2.7 as Kirigami
import org.kde.kosmindoormap 1.0

Kirigami.Page {
    title: map.floorLevels.hasName(map.view.floorLevel) ? map.floorLevels.name(map.view.floorLevel) : ("Floor " + map.floorLevels.name(map.view.floorLevel));
    property point coordinate
    property alias map: map
    property alias debug: infoModel.debug

    topPadding: 0
    bottomPadding: 0
    leftPadding: 0
    rightPadding: 0

    // prevent swipe to the right changing pages, we want to pan the map instead
    // TODO in theory we could make this conditional to having panned the map all the way to the right
    Kirigami.ColumnView.preventStealing: true

    actions {
        left: Kirigami.Action {
            iconName: "go-down-symbolic"
            enabled: map.floorLevels.hasFloorLevelBelow(map.view.floorLevel)
            onTriggered: map.view.floorLevel = map.floorLevels.floorLevelBelow(map.view.floorLevel)
        }
        right: Kirigami.Action {
            iconName: "go-up-symbolic"
            enabled: map.floorLevels.hasFloorLevelAbove(map.view.floorLevel)
            onTriggered: map.view.floorLevel = map.floorLevels.floorLevelAbove(map.view.floorLevel)
        }
    }

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
            text: "Elevator"
        }
        ListView {
            model: floorLevelChangeModel
            delegate: Kirigami.BasicListItem {
                highlighted: false
                text: model.display
                bold: model.isCurrentFloor
                onClicked: {
                    elevatorSheet.sheetOpen = false;
                    map.view.floorLevel = model.floorLevel;
                }
            }
        }
    }

    IndoorMap {
        id: map
        anchors.fill: parent

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
                elevatorSheet.sheetOpen = true;
                return;
            }

            infoModel.element = element;
            if (infoModel.name != "" || infoModel.debug) {
                elementDetailsSheet.sheetOpen = true;
            }
        }
        onElementLongPressed: {
            // this provides info model access for elements with other interactions
            infoModel.element = element;
            if (infoModel.name != "" || infoModel.debug) {
                elementDetailsSheet.sheetOpen = true;
            }
        }
    }

    onCoordinateChanged: map.mapLoader.loadForCoordinate(coordinate.y, coordinate.x);
}
