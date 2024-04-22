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
    title: i18nc("@title:page", "Data Sources")

    required property var publicTransportManager

    Component {
        id: attributionDelegate
        QQC2.ItemDelegate {
            width: ListView.view.width
            onClicked: Qt.openUrlExternally(modelData.url)
            contentItem: Kirigami.TitleSubtitle {
                title: modelData.name
                subtitle: (modelData.license.length > 0 || modelData.licenseUrl.length > 0) ? i18n("License: <a href=\"%1\">%2</a>", modelData.licenseUrl, modelData.license.length > 0 ? modelData.license : modelData.licenseUrl) : i18n("License: Other")
                onLinkActivated: link => Qt.openUrlExternally(link)
            }
        }
    }

    ListView {
        model: publicTransportManager.attributions
        delegate: attributionDelegate
    }
}
