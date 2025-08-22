/*
    SPDX-FileCopyrightText: 2024 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

pragma ComponentBehavior: Bound

import QtQuick
import org.kde.kirigami as Kirigami
import org.kde.kosmindoormap

/** Amenity search dialog.
 *  For use in combination with AmenityModel.
 */
Kirigami.SearchDialog {
    id: root

    property AmenityModel amenityModel: null

    searchFieldPlaceholderText: i18nd("kosmindoormap", "Search amenity…")

    model: AmenitySortFilterProxyModel {
        id: amenitySortModel
        sourceModel: root.visible ? root.amenityModel : null
        filterCaseSensitivity: Qt.CaseInsensitive
    }

    /** Emitted when an entry of this dialog as been selected. */
    signal amenitySelected(amenity: var)

    delegate: AmenityListDelegate {
        id: item
        required property osmElement element
        required property int level
        onClicked: {
            root.amenitySelected({ element: item.element, level: item.level });
            root.accept();
        }

        Keys.onPressed: (event) => {
            if (event.key === Qt.Key_Enter || event.key == Qt.Key_Return) {
                event.accepted = true;
                root.amenitySelected({ element: item.element, level: item.level });
                root.accept();
            }
        }
    }

    section {
        property: "groupName"
        delegate: Kirigami.ListSectionHeader {
            required property string section
            text: section
            width: ListView.view.width
        }
    }

    onTextChanged: amenitySortModel.filterString = root.text
}
