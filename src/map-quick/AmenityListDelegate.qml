/*
    SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.kopeninghours

/** Delegate for use on an AmenityModel. */
QQC2.ItemDelegate {
    id: root
    required property string name
    required property string typeName
    required property string iconSource
    required property string detailsLabel
    required property string fallbackName
    required property string openingHours
    required property point coordinate
    required property string timeZone
    required property string regionCode

    required property int index // for Kirigami

    highlighted: false
    width: ListView.view.width

    property var oh: {
        let v = OpeningHoursParser.parse(root.openingHours);
        v.region = root.regionCode;
        v.timeZone = root.timeZone;
        v.setLocation(root.coordinate.y, root.coordinate.x);
        if (v.error != OpeningHours.Null && v.error != OpeningHours.NoError) {
            console.log("Opening hours parsing error:", root.openingHours, v.error, root.regionCode, root.timeZone)
        }
        return v;
    }

    contentItem: RowLayout {
        spacing: Kirigami.Units.largeSpacing

        Kirigami.Icon {
            Layout.minimumHeight: Kirigami.Units.iconSizes.medium
            Layout.maximumHeight: Kirigami.Units.iconSizes.medium
            Layout.minimumWidth: Kirigami.Units.iconSizes.medium
            Layout.maximumWidth: Kirigami.Units.iconSizes.medium
            source: root.iconSource
            isMask: true
        }

        ColumnLayout {
            Layout.alignment: Qt.AlignVCenter
            Layout.fillWidth: true
            spacing: 0

            QQC2.Label {
                Layout.alignment: Qt.AlignLeft | Qt.AlignBottom
                Layout.fillWidth: true
                text: root.name !== "" ? root.name : root.typeName
                elide: Text.ElideRight
            }

            QQC2.Label {
                Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                Layout.fillWidth: true
                text: {
                    if (root.detailsLabel && root.name === "")
                        return root.detailsLabel;
                    if (root.detailsLabel)
                        return i18n("%1 (%2)", root.typeName, root.detailsLabel);
                    return root.name === "" && root.fallbackName !== "" ? root.fallbackName : root.typeName;
                }
                elide: Text.ElideRight
                font: Kirigami.Theme.smallFont
                opacity: 0.7
            }

            QQC2.Label {
                Layout.fillWidth: true
                text: Display.currentState(root.oh)
                color: {
                    if (root.highlighted || root.checked || root.down)
                        return Kirigami.Theme.highlightedTextColor
                    const currentInterval = root.oh.interval(new Date());
                    switch (currentInterval.state) {
                        case Interval.Open: return Kirigami.Theme.positiveTextColor;
                        case Interval.Closed: return Kirigami.Theme.negativeTextColor;
                        default: return Kirigami.Theme.textColor;
                    }
                }
                elide: Text.ElideRight
                font: Kirigami.Theme.smallFont
                visible: text !== ""
            }
        }
    }
}
