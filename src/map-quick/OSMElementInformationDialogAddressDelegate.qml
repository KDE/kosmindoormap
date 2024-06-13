/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.contacts as KContacts

/** OSM element info dialog delegate for graphically displaying link entries. */
RowLayout {
    id: root

    /** OSMAddress instance to display. */
    required property var address

    QQC2.Label {
        id: label
        property KContacts.address addr: ({
            country: root.address.country,
            region: root.address.state,
            locality: root.address.city,
            postalCode: root.address.postalCode,
            street: root.address.street
        })
        text: label.addr.formatted(KContacts.KContacts.AddressFormatStyle.MultiLineInternational)
    }
}
