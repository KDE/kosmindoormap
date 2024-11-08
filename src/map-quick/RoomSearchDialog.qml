/*
    SPDX-FileCopyrightText: 2024 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.kosmindoormap

/** Search dialogs for buildings/rooms. */
Kirigami.SearchDialog {
    id: root

    /** Room model instance providing the content for this dialog. */
    property RoomModel roomModel

    /** Emitted when an entry of this dialog as been selected. */
    signal roomSelected(room: var)

    width: Math.min(applicationWindow().width, Kirigami.Units.gridUnit * 24)
    height: Math.min(applicationWindow().height, Kirigami.Units.gridUnit * 32)

    searchFieldPlaceholderText: i18nd("kosmindoormap", "Search room…")
    model: RoomSortFilterProxyModel {
        id: roomSortModel
        sourceModel: root.visible ? root.roomModel : null
        filterCaseSensitivity: Qt.CaseInsensitive
    }

    delegate: QQC2.ItemDelegate {
        id: item
        required property string name
        required property string number
        required property string typeName
        required property string levelLongName
        required property int level
        required property osmElement element

        width: ListView.view.width
        contentItem: Kirigami.TitleSubtitle {
            title: {
                if (item.name === "")
                    return item.number;
                if (item.number === "")
                    return item.name;
                return i18nd("kosmindoormap", "%1 (%2)", item.name, item.number);
            }
            subtitle: {
                if (root.roomModel.buildingCount === 1)
                    return item.typeName;
                if (item.typeName === "")
                    return item.levelLongName;
                return i18nd("kosmindoormap", "%1 (%2)", item.typeName, item.levelLongName);
            }
        }
        onClicked: {
            root.roomSelected({ element: item.element, level: item.level });
            root.accept();
        }

        Keys.onPressed: (event) => {
            if (event.key === Qt.Key_Enter || event.key == Qt.Key_Return) {
                event.accepted = true;
                root.roomSelected({ element: item.element, level: item.level });
                root.accept();
            }
        }
    }

    section.property: root.roomModel.buildingCount !== 1 ? "buildingName" : "levelLongName"
    section.delegate: Kirigami.ListSectionHeader {
        text: section
        width: ListView.view.width
    }

    onTextChanged: roomSortModel.filterString = root.text
}
