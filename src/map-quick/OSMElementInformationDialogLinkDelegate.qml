/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami

/** OSM element info dialog delegate for graphically displaying link entries. */
RowLayout {
    id: root

    /** Key name. */
    required property string keyLabel
    /** Value category. */
    required property int category
    /** Link label of this entry. */
    required property string value
    /** URL of this entry. */
    required property string url

    QQC2.Label {
        visible: root.keyLabel !== ""
        text: root.keyLabel + ":"
        color: root.category === OSMElementInformationModel.DebugCategory ? Kirigami.Theme.disabledTextColor : Kirigami.Theme.textColor
        Layout.alignment: Qt.AlignTop
    }
    QQC2.Label {
        Layout.fillWidth: true
        text: "<a href=\"" + root.url + "\">" + root.value + "</a>"
        color: (row && row.category == OSMElementInformationModel.DebugCategory) ? Kirigami.Theme.disabledTextColor : Kirigami.Theme.textColor
        onLinkActivated: Qt.openUrlExternally(link)
        wrapMode: Text.WordWrap
    }
}
