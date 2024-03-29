/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Controls as QQC2

Item {
    id: root

    implicitHeight: background.height
    implicitWidth: background.width

    property int __margin: 2

    Rectangle {
        id: background
        color: label.palette.base
        opacity: 0.5
        height: label.implicitHeight + 2 * root.__margin
        width: label.implicitWidth
    }

    QQC2.Label {
        text: i18nd("kosmindoormap", "© <a href=\"https://www.openstreetmap.org/copyright\">OpenStreetMap contributors</a>");
        id: label
        font.pointSize: 7;
        anchors.centerIn: background

        onLinkActivated: (link) => Qt.openUrlExternally(link)
    }
}
