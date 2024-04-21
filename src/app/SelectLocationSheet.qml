/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.kpublictransport as PublicTransport
import org.kde.kosmindoormap
import org.kde.kosmindoormap.kpublictransport

Kirigami.Dialog {
    id: root
    property point coordinate;
    property var publicTransportManager

    signal coordinateSelected()

    title: i18nc("@title:dialog", "Select Location")

    width: Math.min(applicationWindow().width, Kirigami.Units.gridUnit * 24)

    leftPadding: Kirigami.Units.largeSpacing
    topPadding: Kirigami.Units.largeSpacing
    rightPadding: Kirigami.Units.largeSpacing
    bottomPadding: Kirigami.Units.largeSpacing

    contentItem: ColumnLayout {
        Kirigami.Heading {
            text: "Examples"
            level: 4
        }

        ExampleLocationModel { id: exampleModel }

        QQC2.ComboBox {
            id: exampleBox
            Layout.fillWidth: true
            model: exampleModel
            popup.z: 999 // workaround for ending up below the overlay sheet
            textRole: "label"
            onCurrentIndexChanged: {
                var obj = exampleModel.get(currentIndex);
                root.coordinate.y = obj.lat;
                root.coordinate.x = obj.lon;
            }
        }

        Kirigami.Heading {
            text: "Coordinate"
            level: 4
        }

        RowLayout {
            QQC2.TextField {
                id: coordField
                placeholderText: "<latitude>, <longitude>"
                Layout.fillWidth: true
                onEditingFinished: function() {
                    parseCoordinate();
                }
                function parseCoordinate() {
                    var c = coordField.text.match(/([\d\.-]+)[,;/ ]+([\d\.-]*)/);
                    if (c) {
                        root.coordinate.y = c[1];
                        root.coordinate.x = c[2];
                    }
                }
            }
            QQC2.ToolButton {
                icon.name: "edit-clear-symbolic"
                onClicked: function() {
                    coordField.clear();
                    coordField.parseCoordinate();
                }
            }
            QQC2.ToolButton {
                icon.name: "edit-paste-symbolic"
                onClicked: function() {
                    coordField.clear();
                    coordField.paste();
                    coordField.parseCoordinate();
                }
            }
        }

        Kirigami.Heading {
            text: "Name"
            level: 4
        }

        QQC2.TextField {
            id: nameField
            placeholderText: "train station name"
            Layout.fillWidth: true
            onEditingFinished: function() {
                locationModel.request.name = nameField.text;
                locationModel.request.backends = [ "un_navitia", "de_db" ];
                locationModel.request.maximumResults = 10;
                locationModel.request.types = PublicTransport.Location.Stop;
            }
        }
        PublicTransport.LocationQueryModel {
            id: locationModel
            manager: root.publicTransportManager
        }
        ListView {
            id: nameSearchResultView
            Layout.fillWidth: true
            clip: true
            implicitHeight: contentHeight
            model: locationModel
            delegate: QQC2.ItemDelegate {
                width: ListView.view.width
                contentItem: Kirigami.TitleSubtitle {
                    title: location.name
                }
            }
            onCurrentIndexChanged: function() {
                var loc = locationModel.data(locationModel.index(nameSearchResultView.currentIndex, 0), PublicTransport.LocationQueryModel.LocationRole);
                if (loc != undefined) {
                    root.coordinate.x = loc.longitude;
                    root.coordinate.y = loc.latitude;
                }
            }
        }
    }

    customFooterActions: [
        Kirigami.Action {
            text: "Show"
            enabled: root.coordinate.x != 0.0 && root.coordinate.x != NaN && root.coordinate.y != 0.0 && root.coordinate != NaN
            onTriggered: {
                console.log(root.coordinate);
                root.close();
                coordinateSelected();
            }
        }
    ]

}
