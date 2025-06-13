/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtCore
import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import QtQuick.Dialogs as Dialogs
import org.kde.kirigami as Kirigami
import org.kde.kpublictransport as PublicTransport
import org.kde.kpublictransport.ui as PublicTransport
import org.kde.kosmindoormap
import org.kde.kosmindoormap.kpublictransport
import org.kde.kosmindoorrouting
import org.kde.osm.editorcontroller
import org.kde.kirigamiaddons.formcard as FormCard

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
                text: i18nc("@action", "Data Sources")
                icon.name: "help-about-symbolic"
                onTriggered: pageStack.pushDialogLayer(attributionPage, {}, {
                    title: i18nc("@title:window", "Data Sources")
                })
            },
            Kirigami.Action {
                id: aboutAction
                text: "About"
                icon.name: "help-about-symbolic"
                onTriggered: pageStack.pushDialogLayer(aboutPage, {}, {
                    title: i18nc("@title:window", "About")
                })
            }
        ]
    }
    contextDrawer: Kirigami.ContextDrawer {
        id: contextDrawer
    }

    Dialogs.FileDialog {
        id: fileDialog
        title: "Open OSM File"
        fileMode: Dialogs.FileDialog.OpenFile
        nameFilters: ["o5m file (*.o5m)", "OSM XML file (*.osm *.xml)", "PBF file (*.osm.pbf)"]
        onAccepted: page.map.mapLoader.loadFromFile(fileDialog.selectedFile);
    }
    Dialogs.FileDialog {
        id: mapcssDialog
        title: "Open MapCSS Stylesheet"
        fileMode: Dialogs.FileDialog.OpenFile
        nameFilters: ["MapCSS stylesheet (*.mapcss)"]
        onAccepted: page.map.styleSheet = mapcssDialog.selectedFile
    }
    PublicTransport.Manager { id: ptMgr }
    Settings {
        id: settings
        property alias debugMode: debugAction.checked
        property alias stylesheet: page.map.styleSheet
        property alias hoverMode: page.mapHoverEnabled
    }

    pageStack.initialPage: IndoorMapPage {
        id: page
        debug: debugAction.checked
        routingController: routingController

        actions: [
            Kirigami.Action {
                text: "Select Location"
                icon.name: "search"
                onTriggered: locationSheet.open()
            },
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
                onTriggered: platformSheet.open()
                visible: !platformModel.isEmpty
            },
            Kirigami.Action {
                id: gateAction
                text: "Find Gate"
                onTriggered: gateSheet.open()
                visible: !gateModel.isEmpty
            },
            Kirigami.Action {
                id: amenityAction
                text: "Find Amenity"
                onTriggered: amenitySheet.open()
            },
            Kirigami.Action {
                id: roomAction
                text: "Find Room"
                onTriggered: roomSheet.open()
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
            },
            Kirigami.Action {
                text: i18n("Enable hover selection")
                icon.name: "followmouse"
                checkable: true
                checked: page.mapHoverEnabled
                onToggled: page.mapHoverEnabled = !page.mapHoverEnabled
            },
            Kirigami.Action {
                text: i18n("Configure routing...")
                icon.name: "settings-configure"
                visible: routingController.available
                onTriggered: {
                    routingSheet.routingProfile = routingController.profile;
                    routingSheet.open();
                }
            }
        ]

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
            routingController.searchRoute();
        }

        PlatformModel {
            id: platformModel
            mapData: page.map.mapData
        }

        PlatformDialog {
            id: platformSheet
            model: platformModel
            onPlatformSelected: (platform) => { page.map.view.centerOn(platform.position, platform.level, 19); }
        }

        GateModel {
            id: gateModel
            mapData: page.map.mapData
        }

        Kirigami.Dialog {
            id: gateSheet

            title: i18nc("@title", "Find Gate")

            width: Math.min(applicationWindow().width, Kirigami.Units.gridUnit * 24)
            height: Math.min(applicationWindow().height, Kirigami.Units.gridUnit * 32)

            contentItem: ListView {
                model: gateModel
                Layout.preferredWidth: Kirigami.Units.gridUnit * 10
                delegate: QQC2.ItemDelegate {
                    highlighted: false
                    width: ListView.view.width
                    contentItem: Kirigami.TitleSubtitle {
                        title: model.display
                    }
                    onClicked: {
                        page.map.view.centerOn(model.coordinate, model.level, 18);
                        gateSheet.close();
                    }
                }
            }
        }

        AmenityModel {
            id: amenityModel
            mapData: page.map.mapData
        }

        AmenitySearchDialog {
            id: amenitySheet
            amenityModel: amenityModel
            onAmenitySelected: (amenity) => {
                page.map.view.centerOn(amenity.element.center, amenity.level, 21);
                console.log(amenity.element.url);
            }
        }

        RoomModel {
            id: roomModel
            mapData: page.map.mapData
        }

        RoomSearchDialog {
            id: roomSheet
            roomModel: roomModel
            onRoomSelected: (room) => {
                page.map.view.centerOn(room.element.center, room.level, 21);
                console.log(room.element.url);
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

        map.equipmentModel: RealtimeEquipmentModel {
            mapData: page.map.mapData
            realtimeModel: locationModel.sourceModel
            onUpdate: routingController.searchRoute()
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

        RoutingController {
            id: routingController
            mapData: page.map.mapData
            elevatorModel: page.map.equipmentModel
        }
        QQC2.BusyIndicator {
            anchors.top: parent.top
            anchors.right: parent.right
            running: routingController.inProgress
        }
        RoutingProfileSheet {
            id: routingSheet
            onApplyRoutingProfile: {
                routingController.profile = routingSheet.routingProfile;
                routingController.searchRoute();
            }
        }

        map.overlaySources: [ gateModel, platformModel, locationModel, map.equipmentModel, routingController.routeOverlay ]
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
            page.queryLiveLocationData();
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
                "version": Application.version,
                "licenses": [
                    {
                        "name": "LGPL 2.0 or later",
                        "spdx": "LGPL-2.0-or-later"
                    }
                ],
                "copyrightStatement": "© 2020-2024 The KDE Community",
                "desktopFileName": "org.kde.kosmindoormap",
                "otherText": ""
            }
        }
    }
}
