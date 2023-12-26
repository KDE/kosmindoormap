/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import org.kde.kosmindoormap
import QtQuick.Controls as QQC2

/** QML item for displaying a train station or airport map. */
Item {
    id: mapRoot

    /** Access to map loading status and progress. */
    property alias mapLoader: map.loader
    /** Path to a MapCSS style sheet used for rendering the map. */
    property alias styleSheet: map.styleSheet
    /** Floor level model. */
    property alias floorLevels: map.floorLevels
    /** Access to the view transformation and floor level selection. */
    property alias view: map.view
    /** There is something preventing displaying a map. */
    property alias hasError: map.hasError
    /** Access to the map data, for feeding into content-specific models. */
    property alias mapData: map.mapData
    /** Access to overlay sources. */
    property alias overlaySources: map.overlaySources
    /** ISO 3166-1/2 country or region code for opening hours interpretation. */
    property alias region: map.region
    /** IANA timezone id for opening hours interpretation. */
    property alias timeZone: map.timeZone

    /** Emitted when a map element has been picked by clicking/tapping on it. */
    signal elementPicked(var element);
    /** Emitted when a map element has been long-pressed. */
    signal elementLongPressed(var element);

    MapItemImpl {
        id: map
        anchors.fill: mapRoot

    }

    Flickable {
        id: flickable
        boundsBehavior: Flickable.StopAtBounds
        clip: true
        interactive: !pinchHandler.active
        contentX: map.view.panX
        contentY: map.view.panY
        contentWidth: map.view.panWidth
        contentHeight: map.view.panHeight
        anchors.fill: parent

        onContentXChanged: {
            if (moving) {
                map.view.panTopLeft(flickable.contentX, flickable.contentY);
                map.update();
            }
        }
        onContentYChanged: {
            if (moving) {
                map.view.panTopLeft(flickable.contentX, flickable.contentY);
                map.update();
            }
        }

        QQC2.ScrollBar.vertical: QQC2.ScrollBar {}
        QQC2.ScrollBar.horizontal: QQC2.ScrollBar {}

        TapHandler {
            id: tapHandler
            acceptedButtons: Qt.LeftButton
            onTapped: function(eventPoint) {
                var root = parent;
                while (root.parent) { root = root.parent; }
                var localPos = map.mapFromItem(root, eventPoint.scenePosition.x, eventPoint.scenePosition.y);
                var element = map.elementAt(localPos.x, localPos.y);
                if (!element.isNull) {
                    elementPicked(element);
                }
            }
            onLongPressed: function() {
                var root = parent;
                while (root.parent) { root = root.parent; }
                var localPos = map.mapFromItem(root, tapHandler.point.scenePosition.x, tapHandler.point.scenePosition.y);
                var element = map.elementAt(localPos.x, localPos.y);
                if (!element.isNull) {
                    elementLongPressed(element);
                }
            }
        }
        PinchHandler {
            id: pinchHandler
            target: null
            property double initialZoom
            onActiveChanged: {
                initialZoom = map.view.zoomLevel
            }
            onActiveScaleChanged: {
                map.view.setZoomLevel(pinchHandler.initialZoom + Math.log2(pinchHandler.activeScale),
                                      Qt.point(pinchHandler.centroid.position.x - flickable.contentX, pinchHandler.centroid.position.y - flickable.contentY));
            }
            xAxis.enabled: false
            yAxis.enabled: false
            minimumRotation: 0.0
            maximumRotation: 0.0
        }
        WheelHandler {
            id: wheelHandler
            target: null
            orientation: Qt.Vertical
            acceptedDevices: PointerDevice.Mouse | PointerDevice.TouchPad
            property double initialZoom: 0.0
            onActiveChanged: {
                wheelHandler.initialZoom = map.view.zoomLevel
                wheelHandler.rotation = 0;
            }
            onRotationChanged: {
                // same scale as in qquickmapgestrurearea.cpp
                map.view.setZoomLevel(wheelHandler.initialZoom + 0.05 * wheelHandler.rotation,
                                      Qt.point(wheelHandler.point.position.x - flickable.contentX, wheelHandler.point.position.y - flickable.contentY));
            }
        }
    }

    Connections {
        target: map.view
        function onTransformationChanged() {
            flickable.contentX = map.view.panX;
            flickable.contentY = map.view.panY;
        }
    }

    QQC2.BusyIndicator {
        anchors.centerIn: parent
        running: map.loader.isLoading
    }

    QQC2.Label {
        anchors.fill: parent
        text: map.errorMessage
        visible: map.hasError
        wrapMode: Text.WordWrap
        horizontalAlignment: Qt.AlignHCenter
        verticalAlignment: Qt.AlignVCenter
    }
}
