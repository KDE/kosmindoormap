/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.kopeninghours as OH

/** OSM element info dialog delegate for graphically displaying opening hours. */
ColumnLayout {
    id: root

    /** The opening hours expression to display. */
    property string openingHours
    /** The ISO 3166-1/2 region code of the location the opening hours expression should be evaluated. */
    property string regionCode
    /** The IANA timezone identifier of the timezone in which the opening hours expression should be evaluated. */
    property string timeZoneId
    /** The latitude of the location the opening hours expression refers to. */
    property double latitude: NaN
    /** The longitude of the location the opening hours expression refers to. */
    property double longitude: NaN

    // internal
    readonly property var oh: {
        let v = OH.OpeningHoursParser.parse(root.openingHours);
        v.region = root.regionCode;
        v.timeZone = root.timeZoneId;
        v.setLocation(root.latitude, root.longitude);
        if (v.error != OH.OpeningHours.NoError && root.openingHours !== "") {
            console.log("Opening hours parsing error:", v.error, root.regionCode, root.timeZoneId)
        }
        return v;
    }

    QQC2.Label {
        property OH.interval currentInterval: root.oh.interval(new Date())

        id: currentState
        text: intervalModel.currentState // TODO we could update this every minute
        color: {
            switch (currentInterval.state) {
                case OH.Interval.Open: return Kirigami.Theme.positiveTextColor;
                case OH.Interval.Closed: return Kirigami.Theme.negativeTextColor;
                default: return Kirigami.Theme.textColor;
            }
        }
        visible: text !== ""
    }

    Component {
        id: intervalDelegate
        Item {
            id: delegateRoot
            property var dayData: model
            implicitHeight: row.implicitHeight
            Row {
                id: row
                QQC2.Label {
                    text: delegateRoot.dayData.shortDayName
                    width: delegateRoot.ListView.view.labelWidth + Kirigami.Units.smallSpacing
                    Component.onCompleted: delegateRoot.ListView.view.labelWidth = Math.max(delegateRoot.ListView.view.labelWidth, implicitWidth)
                    font.bold: delegateRoot.dayData.isToday
                }
                Repeater {
                    model: delegateRoot.dayData.intervals
                    Rectangle {
                        id: intervalBox
                        property OH.interval interval: modelData
                        property color closeColor: Kirigami.Theme.negativeBackgroundColor;
                        color: {
                            switch (interval.state) {
                                case OH.Interval.Open: return Kirigami.Theme.positiveBackgroundColor;
                                case OH.Interval.Closed: return intervalBox.closeColor;
                                case OH.Interval.Unknown: return Kirigami.Theme.neutralBackgroundColor;
                            }
                            return "transparent";
                        }
                        width: {
                            const ratio = (interval.estimatedEnd - interval.begin + interval.dstOffset * 1000) / (24 * 60 * 60 * 1000);
                            return ratio * (delegateRoot.ListView.view.width - delegateRoot.ListView.view.labelWidth - Kirigami.Units.smallSpacing);
                        }
                        height: Kirigami.Units.gridUnit
                        gradient: Gradient {
                            orientation: Gradient.Horizontal
                            GradientStop { position: 0.0; color: intervalBox.color }
                            GradientStop { position: (intervalBox.interval.end - intervalBox.interval.begin) / (intervalBox.interval.estimatedEnd - intervalBox.interval.begin); color: intervalBox.color }
                            GradientStop { position: 1.0; color: intervalBox.interval.hasOpenEndTime ? intervalBox.closeColor : intervalBox.color }
                        }

                        QQC2.Label {
                            id: commentLabel
                            text: intervalBox.interval.comment
                            anchors.centerIn: parent
                            visible: commentLabel.implicitWidth < intervalBox.width
                            font.italic: true
                        }
                    }
                }
            }
            Rectangle {
                id: nowMarker
                property double position: (Date.now() - delegateRoot.dayData.dayBegin) / (24 * 60 * 60 * 1000)
                visible: position >= 0.0 && position < 1.0
                color: Kirigami.Theme.textColor
                width: 2
                height: Kirigami.Units.gridUnit
                x: position * (delegateRoot.ListView.view.width - delegateRoot.ListView.view.labelWidth - Kirigami.Units.smallSpacing)
                    + delegateRoot.ListView.view.labelWidth + Kirigami.Units.smallSpacing
            }
        }
    }

    OH.IntervalModel {
        id: intervalModel
        openingHours: root.oh
        // TODO we could use the layover time here, if available and in the future
        beginDate: intervalModel.beginOfWeek(new Date())
        endDate: new Date(intervalModel.beginDate.getTime() + 7 * 24 * 3600 * 1000)
    }

    FontMetrics {
        id: fm
    }

    ListView {
        id: intervalView
        width: parent.width
        height: contentHeight
        boundsBehavior: Flickable.StopAtBounds
        visible: root.oh.error == OH.OpeningHours.NoError
        model: intervalModel
        delegate: intervalDelegate
        property int labelWidth: 0
        spacing: Kirigami.Units.smallSpacing
        clip: true
        header: Row {
            id: intervalHeader
            property int colCount: (intervalView.width - Kirigami.Units.smallSpacing - intervalView.labelWidth) / fm.advanceWidth(intervalModel.formatTimeColumnHeader(12, 59)) < 8 ? 4 : 8
            property int itemWidth: (intervalHeader.ListView.view.width - intervalHeader.ListView.view.labelWidth - Kirigami.Units.smallSpacing) / colCount
            x: intervalHeader.ListView.view.labelWidth + Kirigami.Units.smallSpacing + intervalHeader.itemWidth/2
            Repeater {
                // TODO we might need to use less when space constrained horizontally
                model: intervalHeader.colCount - 1
                QQC2.Label {
                    text: intervalModel.formatTimeColumnHeader((modelData + 1) * 24/colCount, 0)
                    width: intervalHeader.itemWidth
                    horizontalAlignment: Qt.AlignHCenter
                }
            }
        }
    }

    QQC2.Label {
        id: fallbackLabel
        visible: !intervalView.visible
        text: root.openingHours.replace(/;\s*/g, "\n")
    }
}
