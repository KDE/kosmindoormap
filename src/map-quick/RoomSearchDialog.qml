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
    property var roomModel

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
        required property QtObject model
        width: ListView.view.width
        contentItem: Kirigami.TitleSubtitle {
            title: {
                if (model.name === "")
                    return model.number;
                if (model.number === "")
                    return model.name;
                return i18nd("kosmindoormap", "%1 (%2)", model.name, model.number);
            }
            subtitle: {
                if (root.roomModel.buildingCount === 1)
                    return model.typeName;
                if (model.typeName === "")
                    return model.levelLongName;
                return i18nd("kosmindoormap", "%1 (%2)", model.typeName, model.levelLongName);
            }
        }
        onClicked: {
            root.roomSelected({ element: item.model.element, level: item.model.level });
            root.accept();
        }

        Keys.onPressed: (event) => {
            if (event.key === Qt.Key_Enter || event.key == Qt.Key_Return) {
                event.accepted = true;
                root.roomSelected({ element: item.model.element, level: item.model.level });
                root.accept();
            }
        }
    }

    section.property: root.roomModel.buildingCount !== 1 ? "buildingName" : "levelLongName"
    section.delegate: Kirigami.ListSectionHeader {
        label: section
        width: ListView.view.width
    }

    onTextChanged: roomSortModel.filterString = root.text
}
