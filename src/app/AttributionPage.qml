/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.13
import QtQuick.Layouts 1.11
import QtQuick.Controls 2.1 as QQC2
import org.kde.kirigami 2.4 as Kirigami

Kirigami.ScrollablePage {
    id: root
    title: "Data Sources"
    property var publicTransportManager

    Component {
        id: attributionDelegate
        // Kirigami.AbstractListItem {
        QQC2.ItemDelegate {
            width: ListView.view.width
            highlighted: false
            ColumnLayout {
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
