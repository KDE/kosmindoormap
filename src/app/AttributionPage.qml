/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami

Kirigami.ScrollablePage {
    id: root
    title: "Data Sources"
    property var publicTransportManager

    Component {
        id: attributionDelegate
        QQC2.ItemDelegate {
            width: ListView.view.width
            highlighted: false
            contentItem: ColumnLayout {
                QQC2.Label {
                    Layout.fillWidth: true
                    wrapMode: Text.WordWrap
                    text: "<a href=\"" + modelData.url + "\">" + modelData.name + "</a>"
                    onLinkActivated: Qt.openUrlExternally(link)
                }
                QQC2.Label {
                    Layout.fillWidth: true
                    text: "License: <a href=\"" + modelData.liceseUrl + "\">" + (modelData.license != "" ? modelData.license : modelData.licenseUrl) + "</a>"
                    onLinkActivated: Qt.openUrlExternally(link)
                    visible: modelData.hasLicense
                    wrapMode: Text.WordWrap
                }
            }
        }
    }

    ListView {
        model: publicTransportManager.attributions
        delegate: attributionDelegate
    }
}
