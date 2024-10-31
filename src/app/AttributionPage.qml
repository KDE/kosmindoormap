/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.kpublictransport as KPublicTransport

Kirigami.ScrollablePage {
    id: root
    title: i18nc("@title:page", "Data Sources")

    required property KPublicTransport.Manager publicTransportManager

    Component {
        id: attributionDelegate
        QQC2.ItemDelegate {
            id: delegate
            required property KPublicTransport.attribution modelData
            width: ListView.view.width
            onClicked: Qt.openUrlExternally(delegate.modelData.url)
            contentItem: Kirigami.TitleSubtitle {
                title: delegate.modelData.name
                subtitle: (delegate.modelData.license.length > 0 || delegate.modelData.licenseUrl.length > 0) ? i18n("License: <a href=\"%1\">%2</a>", delegate.modelData.licenseUrl, delegate.modelData.license.length > 0 ? delegate.modelData.license : delegate.modelData.licenseUrl) : i18n("License: Other")
                onLinkActivated: link => Qt.openUrlExternally(link)
            }
        }
    }

    ListView {
        model: root.publicTransportManager.attributions
        delegate: attributionDelegate
    }
}
