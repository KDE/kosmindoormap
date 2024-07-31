/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import QtQuick.Templates as T
import org.kde.kirigami as Kirigami
import org.kde.kosmindoormap

/** Dialog showing detailed information about a specific OSM element.
 *  Level of detail depends on what is tagged in the OSM data.
 */
Kirigami.Dialog {
    id: root

    /** OSMElementInformationModel instance for the element to show. */
    required property var model
    /** Fallback ISO 3166-1/2 region code for interpretting opening hours. */
    required property string regionCode
    /** Fallback IANA time zone id for interpretting opening hours. */
    required property string timeZone

    width: Math.min(applicationWindow().width, Kirigami.Units.gridUnit * 24)
    height: Math.min(applicationWindow().height, Kirigami.Units.gridUnit * 32)
    padding: 0

    // see Kirigami.Dialog
    header: T.Control {
        implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                                implicitContentWidth + leftPadding + rightPadding)
        implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                                implicitContentHeight + topPadding + bottomPadding)

        padding: Kirigami.Units.largeSpacing
        bottomPadding: verticalPadding + headerSeparator.implicitHeight // add space for bottom separator

        Kirigami.Theme.colorSet: Kirigami.Theme.Header

        contentItem: GridLayout {
            rowSpacing: Kirigami.Units.smallSpacing
            columnSpacing: Kirigami.Units.smallSpacing

            Kirigami.Heading {
                Layout.row: 0
                Layout.column: 0
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignVCenter
                text: root.model.name
                elide: Text.ElideRight

                QQC2.ToolTip.visible: truncated && titleHoverHandler.hovered
                QQC2.ToolTip.text: root.model.name
                HoverHandler { id: titleHoverHandler }
            }

            Kirigami.Heading {
                Layout.row: 1
                Layout.column: 0
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignVCenter
                text: root.model.category
                elide: Text.ElideRight
                visible: root.model.category.length > 0
                level: 4

                QQC2.ToolTip.visible: truncated && subtitleHoverHandler.hovered
                QQC2.ToolTip.text: root.model.category
                HoverHandler { id: subtitleHoverHandler }
            }

            QQC2.ToolButton {
                id: closeIcon
                Layout.row: 0
                Layout.column: 1
                Layout.columnSpan: 2
                Layout.alignment: Qt.AlignRight | Qt.AlignTop

                visible: root.showCloseButton
                icon.name: closeIcon.hovered ? "window-close" : "window-close-symbolic"
                text: i18ndc("kosmindoormap", "@action:button close dialog", "Close")
                onClicked: root.reject()
                display: QQC2.AbstractButton.IconOnly
            }
        }

        // header background
        background: Item {
            Kirigami.Separator {
                id: headerSeparator
                width: parent.width
                anchors.bottom: parent.bottom
                visible: contentControl.contentHeight > contentControl.implicitHeight
            }
        }
    }

    contentItem: ListView {
        id: contentView
        model: root.model
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
            OSMElementInformationDialogAddressDelegate {
                x: Kirigami.Units.largeSpacing
                width: parent.ListView.view.width - 2 * x
                address: row?.value ?? ""
            }
        }

        Component {
            id: infoOpeningHoursDelegate
            OSMElementInformationDialogOpeningHoursDelegate {
                x: Kirigami.Units.largeSpacing
                width: parent.ListView.view.width - 2 * x
                regionCode: root.regionCode
                timeZoneId: root.timeZone
                latitude: root.model.element?.center.y ?? NaN
                longitude: root.model.element?.center.x ?? NaN
                openingHours: row?.value ?? ""
            }
        }

        Component {
            id: infoImageDelegate
            OSMElementInformationDialogImageDelegate {
                // logos needs margings, images can be shown full width
                x: (row?.key === OSMElementInformationModel.Logo ?? false) ? Kirigami.Units.largeSpacing : 0
                width: parent.ListView.view.width - 2 * x
                source: row?.value ?? ""
                url: row?.url ?? ""
                maximumHeight: (row?.key === OSMElementInformationModel.Logo ?? false) ? Kirigami.Units.gridUnit * 6 : implicitHeight
                topMargin: x
                bottomMargin: x
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
                    case OSMElementInformationModel.ImageType:
                        return infoImageDelegate;
                    case OSMElementInformationModel.String:
                    default:
                        return infoStringDelegate;
                }
            }
        }
    }

    onClosed: root.model.clear()
}
