/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Controls as QQC2

Item {
    id: root
    property var map

    implicitHeight: background.height
    visible: !map.mapLoader.isLoading && !map.hasError

    property int __margin: 2

    function updateScale() {
        var d = map.view.mapScreenToMeters(background.width - 2 * __margin);
        var s = d < 5 ? 1 : d < 20 ? 5 : d < 100 ? 10 : 20;
        d /= s;
        d = Math.floor(d);
        d *= s;
        scaleLabel.text = i18ndc("kosmindoormap", "length in meters", "%1 m", d)
        scale.width = map.view.mapMetersToScreen(d);
    }

    Rectangle {
        id: background
        color: scaleLabel.palette.base
        opacity: 0.5
        height: scaleLabel.implicitHeight + scale.height + 2 * root.__margin
        width: root.width
    }

    QQC2.Label {
        id: scaleLabel
        anchors.bottom: scale.top
        anchors.horizontalCenter: scale.horizontalCenter
    }

    Rectangle {
        id: scale
        anchors.bottom: root.bottom
        anchors.left: root.left
        anchors.margins: root.__margin
        height: 4
        color: scaleLabel.color
    }

    Component.onCompleted: root.updateScale()

    Connections {
        target: root.map.view
        function onTransformationChanged() { root.updateScale(); }
    }
    Connections {
        target: root
        function onWidthChanged() { root.updateScale(); }
    }
}
