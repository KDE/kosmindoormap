/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.kosmindoormap
import org.kde.osm.editorcontroller

Kirigami.Dialog {
    id: elementDetailsSheet

    required property var model
    required property string regionCode
    required property string timeZone

    width: Math.min(applicationWindow().width, Kirigami.Units.gridUnit * 24)
    height: Math.min(applicationWindow().height, Kirigami.Units.gridUnit * 32)

    title: elementDetailsSheet.model.name + (elementDetailsSheet.model.category.length > 0 ? (" - " + elementDetailsSheet.model.category) : "")

    contentItem: ListView {
        id: contentView
        model: elementDetailsSheet.model
        clip: true
        Layout.preferredWidth: Kirigami.Units.gridUnit * 25

        Kirigami.PlaceholderMessage {
            visible: contentView.count === 0
            text: i18nc("@info", "No information available")
            anchors.centerIn: parent
        }

        Component {
            id: infoStringDelegate
            OSMElementInformationDialogStringDelegate {
                x: Kirigami.Units.largeSpacing
                width: parent.ListView.view.width - 2 * x
                keyLabel: row?.keyLabel ?? ""
                value: row?.value ?? ""
                category: row?.category ?? -1
            }
        }

        Component {
            id: infoLinkDelegate
            OSMElementInformationDialogLinkDelegate {
                x: Kirigami.Units.largeSpacing
                width: parent.ListView.view.width - 2 * x
                keyLabel: row?.keyLabel ?? ""
                value: row?.value ?? ""
                category: row?.category ?? -1
                url: row?.url ?? ""
            }
        }

        Component {
            id: infoAddressDelegate
            QQC2.Label {
                x: Kirigami.Units.largeSpacing
                width: parent.ListView.view.width - 2 * x
                text: (row.value.street + " " + row.value.houseNumber + "\n" + row.value.postalCode + " " + row.value.city + "\n" + row.value.country).trim()
            }
        }

        Component {
            id: infoOpeningHoursDelegate
            OSMElementInformationDialogOpeningHoursDelegate {
                x: Kirigami.Units.largeSpacing
                width: parent.ListView.view.width - 2 * x
                regionCode: elementDetailsSheet.regionCode
                timeZoneId: elementDetailsSheet.timeZone
                latitude: elementDetailsSheet.model.element?.center.y ?? NaN
                longitude: elementDetailsSheet.model.element?.center.x ?? NaN
                openingHours: row?.value ?? ""
            }
        }

        section.property: "categoryLabel"
        section.delegate: Kirigami.Heading {
            x: Kirigami.Units.largeSpacing
            level: 4
            text: section
            color: section == "Debug" ? Kirigami.Theme.disabledTextColor : Kirigami.Theme.textColor
            height: implicitHeight + Kirigami.Units.largeSpacing
            verticalAlignment: Qt.AlignBottom
        }
        section.criteria: ViewSection.FullString
        section.labelPositioning: ViewSection.InlineLabels

        delegate: Loader {
            property var row: model
            sourceComponent: {
                switch (row.type) {
                    case OSMElementInformationModel.Link:
                        return infoLinkDelegate;
                    case OSMElementInformationModel.PostalAddress:
                        return infoAddressDelegate;
                    case OSMElementInformationModel.OpeningHoursType:
                        return infoOpeningHoursDelegate;
                    case OSMElementInformationModel.String:
                    default:
                        return infoStringDelegate;
                }
            }
        }
    }

    customFooterActions: [
        Kirigami.Action {
            icon.name: "document-edit"
            text: "Edit with iD"
            onTriggered: EditorController.editElement(elementDetailsSheet.model.element.element, Editor.ID)
        },
        Kirigami.Action {
            icon.name: "org.openstreetmap.josm"
            text: "Edit with JOSM"
            visible: EditorController.hasEditor(Editor.JOSM)
            onTriggered: EditorController.editElement(elementDetailsSheet.model.element.element, Editor.JOSM)
        },
        Kirigami.Action {
            icon.name: "document-edit"
            text: "Edit with Vespucci"
            visible: EditorController.hasEditor(Editor.Vespucci)
            onTriggered: EditorController.editElement(elementDetailsSheet.model.element.element, Editor.Vespucci)
        }
    ]

    onClosed: elementDetailsSheet.model.clear()
}
