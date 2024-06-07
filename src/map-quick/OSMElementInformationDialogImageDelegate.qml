/*
    SPDX-FileCopyrightText: 2024 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick

Image {
    id: root
    required property string url

    fillMode: Image.PreserveAspectFit

    // weird, but TapHandler doesn't forward unhandled drags to our parent ListView but the map area behind this dialog...
    MouseArea {
        anchors.fill: root
        onClicked:  Qt.openUrlExternally(root.url);
        HoverHandler {
            cursorShape: Qt.PointingHandCursor
        }
    }
}
