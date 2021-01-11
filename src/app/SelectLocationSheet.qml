/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.12
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.1 as QQC2
import Qt.labs.platform 1.0 as QPlatform
import org.kde.kirigami 2.0 as Kirigami
import org.kde.kpublictransport 1.0 as PublicTransport
import org.kde.kosmindoormap 1.0
import org.kde.kosmindoormap.kpublictransport 1.0

Kirigami.OverlaySheet {
    id: root
    property point coordinate;
    property var publicTransportManager

    signal coordinateSelected()

    header: Kirigami.Heading {
        text: "Select Location"
    }

    ColumnLayout {
        Kirigami.Heading {
            text: "Examples"
            level: 4
        }

        ExampleLocationModel { id: exampleModel }

        QQC2.ComboBox {
            id: exampleBox
            Layout.fillWidth: true
            model: exampleModel
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
            delegate: Kirigami.BasicListItem {
                label: location.name
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

    footer: QQC2.Button {
        text: "Show"
        enabled: root.coordinate.x != 0.0 && root.coordinate.x != NaN && root.coordinate.y != 0.0 && root.coordinate != NaN
        onClicked: {
            console.log(root.coordinate);
            root.sheetOpen = false;
            coordinateSelected();
        }
    }
}
