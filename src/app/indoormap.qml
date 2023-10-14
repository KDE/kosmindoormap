/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15 as QQC2
import Qt.labs.platform 1.0 as QPlatform
import Qt.labs.settings 1.0 as QSettings
import org.kde.kirigami 2.20 as Kirigami
import org.kde.kpublictransport 1.0 as PublicTransport
import org.kde.kosmindoormap 1.0
import org.kde.kosmindoormap.kpublictransport 1.0
import org.kde.osm.editorcontroller 1.0
import org.kde.kirigamiaddons.formcard 1.0 as FormCard

Kirigami.ApplicationWindow {
    globalDrawer: Kirigami.GlobalDrawer {
        title: "Indoor Map"
        titleIcon: "map-symbolic"
        isMenu: true
        actions: [
            Kirigami.Action {
                text: "Open O5M File"
                icon.name: "document-open-symbolic"
                onTriggered: fileDialog.open()
            },
            Kirigami.Action {
                text: "Open MapCSS Stylesheet"
                icon.name: "document-open-symbolic"
                onTriggered: mapcssDialog.open()
            },
            Kirigami.Action {
                text: "Reload Stylesheet"
                icon.name: "view-refresh-symbolic"
                shortcut: "F5"
                onTriggered: {
                    var s = page.map.styleSheet;
                    page.map.styleSheet = "";
                    page.map.styleSheet = s;
                }
            },
            Kirigami.Action { separator: true },
            Kirigami.Action {
                text: "Data Sources"
                icon.name: "help-about-symbolic"
                onTriggered: function() { applicationWindow().pageStack.push(attributionPage); }
            },
            Kirigami.Action {
                id: aboutAction
                text: "About"
                icon.name: "help-about-symbolic"
                onTriggered: function() { applicationWindow().pageStack.push(aboutPage); }
            }
        ]
    }
    contextDrawer: Kirigami.ContextDrawer {
        id: contextDrawer
    }

    QPlatform.FileDialog {
        id: fileDialog
        title: "Open OSM File"
        fileMode: QPlatform.FileDialog.OpenFile
        nameFilters: ["o5m file (*.o5m)", "OSM XML file (*.osm *.xml)", "PBF file (*.osm.pbf)"]
        onAccepted: page.map.mapLoader.loadFromFile(fileDialog.file);
    }
    QPlatform.FileDialog {
        id: mapcssDialog
        title: "Open MapCSS Stylesheet"
        fileMode: QPlatform.FileDialog.OpenFile
        nameFilters: ["MapCSS stylesheet (*.mapcss)"]
        onAccepted: page.map.styleSheet = mapcssDialog.file
    }
    PublicTransport.Manager { id: ptMgr }
    QSettings.Settings {
        id: settings
        property alias debugMode: debugAction.checked
        property alias stylesheet: page.map.styleSheet
    }

    pageStack.initialPage: IndoorMapPage {
        id: page
        debug: debugAction.checked

        actions {
            main: Kirigami.Action {
                text: "Select Location"
                icon.name: "search"
                onTriggered: locationSheet.sheetOpen = true
            }
            contextualActions: [
                Kirigami.Action {
                    text: "Light Style"
                    onTriggered: page.map.styleSheet = "breeze-light"
                },
                Kirigami.Action {
                    text: "Dark Style"
                    onTriggered: page.map.styleSheet = "breeze-dark"
                },
                Kirigami.Action {
                    text: "Diagnostic View"
                    onTriggered: page.map.styleSheet = "diagnostic"
                },
                Kirigami.Action {
                    id: debugAction
                    text: "Debug Info Model"
                    checkable: true
                    checked: false
                },
                Kirigami.Action {
                    id: platformAction
                    text: "Find Platform"
                    onTriggered: platformSheet.sheetOpen = true
                    visible: !platformModel.isEmpty
                },
                Kirigami.Action {
                    id: gateAction
                    text: "Find Gate"
                    onTriggered: gateSheet.sheetOpen = true
                    visible: !gateModel.isEmpty
                },
                Kirigami.Action {
                    id: amenityAction
                    text: "Find Amenity"
                    onTriggered: amenitySheet.open()
                },
                Kirigami.Action {
                    id: equipmentAction
                    text: "Show Elevator Status"
                    checkable: true
                    enabled: !page.map.mapLoader.isLoading
                    onTriggered: page.queryLiveLocationData();
                },
                Kirigami.Action {
                    id: rentalVehicleAction
                    text: i18n("Show Rental Vehicles")
                    checkable: true
                    enabled: !page.map.mapLoader.isLoading
                    onTriggered: page.queryLiveLocationData();
                },
                Kirigami.Action {
                    text: i18n("Edit with iD")
                    icon.name: "document-edit"
                    onTriggered: EditorController.editBoundingBox(page.map.view.mapSceneToGeo(page.map.view.viewport), Editor.ID)
                },
                Kirigami.Action {
                    text: i18n("Edit with JOSM")
                    icon.name: "org.openstreetmap.josm"
                    visible: EditorController.hasEditor(Editor.JOSM)
                    onTriggered: EditorController.editBoundingBox(page.map.view.mapSceneToGeo(page.map.view.viewport), Editor.JOSM)
                },
                Kirigami.Action {
                    text: i18n("Edit with Vespucci")
                    icon.name: "document-edit"
                    visible: EditorController.hasEditor(Editor.Vespucci)
                    onTriggered: EditorController.editBoundingBox(page.map.view.mapSceneToGeo(page.map.view.viewport), Editor.Vespucci)
                }

            ]
        }

        function queryLiveLocationData() {
            if (rentalVehicleAction.checked || equipmentAction.checked) {
                locationQuery.request.latitude = map.mapData.center.y;
                locationQuery.request.longitude = map.mapData.center.x;
                locationQuery.request.maximumDistance = map.mapData.radius;
                locationQuery.request.types =
                    (rentalVehicleAction.checked ? (PublicTransport.Location.RentedVehicleStation | PublicTransport.Location.RentedVehicle) : 0)
                | (equipmentAction.checked ? PublicTransport.Location.Equipment : 0);
            } else {
                locationQuery.clear();
            }
        }

        PlatformModel {
            id: platformModel
            mapData: page.map.mapData
        }

        Component {
            id: platformDelegate
            Kirigami.AbstractListItem {
                property var platform: model
                Row {
                    QQC2.Label { text: platform.lines.length == 0 ? platform.display : (platform.display + " - "); }
                    Repeater {
                        model: platform.lines
                        delegate: Row {
                            Kirigami.Icon {
                                id: icon
                                height: Kirigami.Units.iconSizes.small
                                width: implicitWidth
                                visible: source != ""
                                source: {
                                    switch (platform.mode) {
                                        case Platform.Rail:
                                            return PublicTransport.LineMetaData.lookup(modelData, platform.coordinate.y, platform.coordinate.x, PublicTransport.Line.Train, true).logo;
                                        case Platform.Tram:
                                            return PublicTransport.LineMetaData.lookup(modelData, platform.coordinate.y, platform.coordinate.x, PublicTransport.Line.Tramway, true).logo;
                                        case Platform.Subway:
                                            return PublicTransport.LineMetaData.lookup(modelData, platform.coordinate.y, platform.coordinate.x, PublicTransport.Line.Metro, true).logo;
                                    }
                                    return "";
                                }
                            }
                            QQC2.Label {
                                text: modelData + " "
                                visible: icon.source == ""
                            }
                        }
                    }
                }
                highlighted: false
                onClicked: {
                    page.map.view.floorLevel = model.level
                    page.map.view.centerOnGeoCoordinate(model.coordinate);
                    page.map.view.setZoomLevel(19, Qt.point(page.map.width / 2.0, page.map.height/ 2.0));
                    platformSheet.sheetOpen = false
                }
            }
        }

        Kirigami.OverlaySheet {
            id: platformSheet

            header: Kirigami.Heading {
                text: "Find Platform"
            }

            ListView {
                model: platformModel

                section.property: "mode"
                section.delegate: Kirigami.Heading {
                    x: Kirigami.Units.largeSpacing
                    level: 4
                    text: switch(parseInt(section)) {
                        case Platform.Rail: return "Railway";
                        case Platform.Subway: return "Subway";
                        case Platform.Tram: return "Tramway";
                        case Platform.Bus: return "Bus";
                        default: console.log(section, Platform.Rail); return section;
                    }
                    height: implicitHeight + Kirigami.Units.largeSpacing
                    verticalAlignment: Qt.AlignBottom
                }
                section.criteria: ViewSection.FullString
                section.labelPositioning: ViewSection.CurrentLabelAtStart | ViewSection.InlineLabels

                delegate: platformDelegate
            }
        }

        GateModel {
            id: gateModel
            mapData: page.map.mapData
        }

        Kirigami.OverlaySheet {
            id: gateSheet

            header: Kirigami.Heading {
                text: "Find Gate"
            }

            ListView {
                model: gateModel

                delegate: Kirigami.BasicListItem {
                    text: model.display
                    highlighted: false
                    onClicked: {
                        page.map.view.floorLevel = model.level
                        page.map.view.centerOnGeoCoordinate(model.coordinate);
                        page.map.view.setZoomLevel(18, Qt.point(page.map.width / 2.0, page.map.height/ 2.0));
                        gateSheet.sheetOpen = false
                    }
                }
            }
        }

        AmenityModel {
            id: amenityModel
            mapData: page.map.mapData
        }

        Kirigami.OverlaySheet {
            id: amenitySheet
            header: Kirigami.Heading {
                text: "Find Amenity"
            }

            ListView {
                model: AmenitySortFilterProxyModel {
                    sourceModel: amenitySheet.sheetOpen ? amenityModel : null
                    filterCaseSensitivity: Qt.CaseInsensitive
                    filterString: amenitySearchField.text
                }

                delegate: IndoorMapAmenityDelegate {
                    id: item
                    mapData: page.map.mapData
                    required property QtObject model
                    onClicked: {
                        page.map.view.floorLevel = item.model.level
                        page.map.view.centerOnGeoCoordinate(item.model.coordinate);
                        page.map.view.setZoomLevel(21, Qt.point(page.map.width / 2.0, page.map.height/ 2.0));
                        console.log(item.model.element.url);
                        amenitySheet.close();
                    }
                }

                section.property: "groupName"
                section.delegate: Kirigami.ListSectionHeader {
                    label: section
                }
            }

            footer: Kirigami.SearchField {
                id: amenitySearchField
                focus: true
            }
            onSheetOpenChanged: {
                if (sheetOpen)
                    amenitySearchField.clear();
            }
        }

        LocationQueryOverlayProxyModel {
            id: locationModel
            sourceModel: PublicTransport.LocationQueryModel {
                id: locationQuery
                manager: ptMgr
            }
            mapData: page.map.mapData
        }

        RealtimeEquipmentModel {
            id: equipmentModel
            mapData: page.map.mapData
            realtimeModel: locationModel.sourceModel
        }

        SelectLocationSheet {
            id: locationSheet
            publicTransportManager: ptMgr
            onCoordinateSelected: function() {
                page.map.mapLoader.loadForCoordinate(locationSheet.coordinate.y, locationSheet.coordinate.x);
                page.map.view.beginTime = new Date();
                page.map.view.endTime = new Date(page.map.view.beginTime.getTime() + 3600000);
                // TODO timezone

                settings.setValue("latitude", locationSheet.coordinate.y);
                settings.setValue("longitude", locationSheet.coordinate.x);
            }
        }

        map.overlaySources: [ gateModel, platformModel, locationModel, equipmentModel ]
        map.timeZone: "Europe/Berlin"

        header: RowLayout {
            QQC2.Label { text: "Floor Level:" }
            QQC2.ComboBox {
                id: floorLevelCombo
                model: page.map.floorLevels
                textRole: "display"
                currentIndex: page.map.floorLevels.rowForLevel(page.map.view.floorLevel);
                onCurrentIndexChanged: if (currentIndex >= 0) { page.map.view.floorLevel = page.map.floorLevels.levelForRow(currentIndex); }
                Layout.fillWidth: true
            }
            Connections {
                target: page.map.view
                function onFloorLevelChanged() { floorLevelCombo.currentIndex = page.map.floorLevels.rowForLevel(page.map.view.floorLevel); }
            }

            QQC2.Slider {
                id: zoomSlider
                from: 14.0
                to: 21.0
                live: true
                Layout.fillWidth: true

                onValueChanged: {
                    page.map.view.setZoomLevel(value, Qt.point(page.map.width / 2.0, page.map.height/ 2.0));
                }
            }
            Connections {
                target: page.map.view
                function onZoomLevelChanged() { zoomSlider.value = page.map.view.zoomLevel; }
            }
        }

        coordinate: Qt.point(settings.value("longitude", 11.08196), settings.value("latitude", 49.44572))
    }

    Connections {
        target: page.map
        function onMapDataChanged() {
            queryLiveLocationData();
        }
    }

    Component {
        id: attributionPage
        AttributionPage {
            publicTransportManager: ptMgr
        }
    }

    Component {
        id: aboutPage
        FormCard.AboutPage {
            aboutData: {
                "displayName": "KDE OSM Indoor Map",
                "productName": "org.kde.kosmindoormap",
                "componentName": "org.kde.kosmindoormap",
                "shortDescription": "OSM Indoor Map Demo",
                "homepage": "https://kde.org/",
                "bugAddress": "submit@bugs.kde.org",
                "version": "23.08",
                "licenses": [
                    {
                        "name": "LGPL 2.0 or later",
                        "spdx": "LGPL-2.0-or-later"
                    }
                ],
                "copyrightStatement": "© 2020-2023 The KDE Team",
                "desktopFileName": "kosmindoormap"
            }
        }
    }
}
