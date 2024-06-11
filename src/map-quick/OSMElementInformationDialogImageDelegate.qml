/*
    SPDX-FileCopyrightText: 2024 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick

/** Delegate for showing images and logos in the OSM element information dialog. */
Item {
    id: root

    /** URL to link to for attribution/source information. */
    required property string url
    /** URL of the image to show. */
    property alias source: img.source
    /** Maximum height for the image, scale down if the source is heigher than this. */
    property int maximumHeight: implicitHeight

    property alias topMargin: img.y
    property real bottomMargin: 0


    implicitHeight: img.implicitHeight + root.topMargin + root.bottomMargin
    implicitWidth: img.implicitWidth
    height: img.height + root.topMargin + root.bottomMargin

    Image {
        id: img

        anchors.horizontalCenter: root.horizontalCenter
        width: Math.min(root.width, img.implicitWidth)
        height: Math.min(img.width / img.implicitWidth * img.implicitHeight, root.maximumHeight)

        fillMode: Image.PreserveAspectFit

        // weird, but TapHandler doesn't forward unhandled drags to our parent ListView but the map area behind this dialog...
        MouseArea {
            anchors.fill: img
            onClicked:  Qt.openUrlExternally(root.url);
            HoverHandler {
                cursorShape: Qt.PointingHandCursor
            }
        }
    }
}
