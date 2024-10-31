/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

pragma ValueTypeBehavior: Addressable

import QtQuick
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
    /** Currently hovered element. */
    property alias hoveredElement: map.hoveredElement

    /** Emitted when a map element has been picked by clicking/tapping on it.
     *  @deprecated Use tapped() instead.
     */
    signal elementPicked(var element);
    /** Emitted when a map element has been long-pressed.
     *  @deprecated Use longPressed() instead.
     */
    signal elementLongPressed(var element);

    /** Emitted on a tap or click event on the map. */
    signal tapped(mapPointerEvent event);
    /** Emitted on a long press event on the map. */
    signal longPressed(mapPointerEvent event);

    /** Map an event handler EventPoint to map screen coordinates. */
    function mapEventPointToScreen(eventPoint) {
        let root = mapRoot.parent;
        while (root.parent) { root = root.parent; }
        return map.mapFromItem(root, eventPoint.scenePosition.x, eventPoint.scenePosition.y);
    }
    /** Map an event handler EventPoint to geo coordinates. */
    function mapEventPointToGeo(eventPoint) {
        return map.view.mapSceneToGeoPoint(map.view.mapScreenToScene(mapRoot.mapEventPointToScreen(eventPoint)));
    }

    /** Returns the OSM element at the given screen position, if any. */
    function elementAt(screenPosition) {
        return map.elementAt(screenPosition.x, screenPosition.y);
    }

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
            acceptedButtons: Qt.LeftButton | Qt.RightButton
            onTapped: (eventPoint, button) => {
                const ev = {
                    element: mapRoot.elementAt(mapRoot.mapEventPointToScreen(eventPoint)),
                    geoPosition: mapRoot.mapEventPointToGeo(eventPoint),
                    screenPosition: mapRoot.mapEventPointToScreen(eventPoint),
                    button: button,
                    modifiers: tapHandler.point.modifiers,
                } as mapPointerEvent;
                if (!ev.element.isNull) {
                    mapRoot.elementPicked(ev.element);
                }
                mapRoot.tapped(ev);
            }
            onLongPressed: function() {
                const ev = {
                    element: mapRoot.elementAt(mapRoot.mapEventPointToScreen(tapHandler.point)),
                    geoPosition: mapRoot.mapEventPointToGeo(tapHandler.point),
                    screenPosition: mapRoot.mapEventPointToScreen(tapHandler.point),
                    button: tapHandler.point.pressedButtons,
                    modifiers: tapHandler.point.modifiers,
                } as mapPointerEvent;
                if (!ev.element.isNull) {
                    mapRoot.elementLongPressed(ev.element);
                }
                mapRoot.longPressed(ev);
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
