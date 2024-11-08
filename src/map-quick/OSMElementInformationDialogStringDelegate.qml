/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.kosmindoormap

/** OSM element info dialog delegate for graphically displaying textual entries. */
RowLayout {
    id: root

    /** Key name. */
    required property string keyLabel
    /** Value category. */
    required property int category
    /** Value of this entry. */
    required property string value

    QQC2.Label {
        visible: root.keyLabel !== ""
        text: root.keyLabel + ":"
        color: root.category === OSMElementInformationModel.DebugCategory ? Kirigami.Theme.disabledTextColor : Kirigami.Theme.textColor
        Layout.alignment: Qt.AlignTop
    }
    QQC2.Label {
        text: root.value
        color: root.category === OSMElementInformationModel.DebugCategory ? Kirigami.Theme.disabledTextColor : Kirigami.Theme.textColor
        wrapMode: Text.WordWrap
        Layout.fillWidth: true
    }
}
