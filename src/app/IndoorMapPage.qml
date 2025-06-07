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
    property alias debug: map.elementInfoModel.debug
    property alias mapHoverEnabled: hoverHandler.enabled
    property RoutingController routingController

    topPadding: 0
    bottomPadding: 0
    leftPadding: 0
    rightPadding: 0

    // prevent swipe to the right changing pages, we want to pan the map instead
    // TODO in theory we could make this conditional to having panned the map all the way to the right
    Kirigami.ColumnView.preventStealing: true

    IndoorMapView {
        id: map
        anchors.fill: parent
        hoveredElement: map.elementAt(map.mapEventPointToScreen(hoverHandler.point))

        elementInfoModel {
            allowOnlineContent: true
            debug: true
        }

        elementInfoDialog: IndoorMapInfoSheet {
            model: map.elementInfoModel
            regionCode: map.mapData.regionCode
            timeZone: map.mapData.timeZone
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
                // enabled: !ev.element.isNull && (map.elementInfoModel.name !== "" || map.elementInfoModel.debug)
                text: i18n("Show information")
                icon.name: "documentinfo"
                onTriggered: map.elementInfoDialog.open()
            }
        }

        function showContextMenu(ev) {
            map.elementInfoModel.element = ev.element;
            contextMenuInfoAction.enabled = !ev.element.isNull && (map.elementInfoModel.name !== "" || map.elementInfoModel.debug);
            contextMenu.ev = ev;
            contextMenu.popup(map, ev.screenPosition);
        }

        HoverHandler {
            id: hoverHandler
            enabled: false
            acceptedDevices: PointerDevice.Mouse | PointerDevice.TouchPad
        }
    }

    onCoordinateChanged: map.mapLoader.loadForCoordinate(coordinate.y, coordinate.x);
}
