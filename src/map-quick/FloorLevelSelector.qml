/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.kosmindoormap

/** Floor level selection popup, e.g. for elevators. */
Kirigami.Dialog {
    id: root

    /** Instance of a FloorLevelChangeModel. */
    property FloorLevelChangeModel model

    /** Emitted when a floor level has been selected. */
    signal floorLevelSelected(level: int)

    title: root.model.title

    width: Math.min(applicationWindow().width, Kirigami.Units.gridUnit * 10)
    height: Math.min(applicationWindow().height, listView.contentHeight + root.header.implicitHeight + Kirigami.Units.largeSpacing)

    contentItem: ListView {
        id: listView
        clip: true
        model: root.model
        keyNavigationEnabled: true

        delegate: QQC2.ItemDelegate {
            id: delegateRoot
            required property string name
            required property int floorLevel
            required property bool isCurrentFloor

            width: ListView.view.width
            contentItem: Kirigami.TitleSubtitle {
                title: delegateRoot.name
                font.bold: delegateRoot.isCurrentFloor
            }
            onClicked: {
                root.close();
                root.floorLevelSelected(delegateRoot.floorLevel);
            }
            Keys.onPressed: (event) => {
                if (event.key === Qt.Key_Enter || event.key == Qt.Key_Return) {
                    event.accepted = true;
                    root.close();
                    root.floorLevelSelected(delegateRoot.floorLevel);
                }
            }
        }
    }

    onOpened: {
        listView.forceActiveFocus();
        listView.currentIndex = root.model.currentFloorLevelRow;
    }
}
