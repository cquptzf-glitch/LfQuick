import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts

Controls.Control {
    id: root

    property var model: []
    property int currentIndex: -1

    function titleFor(value) {
        if (typeof value === "string")
            return value
        return value && value.title !== undefined ? String(value.title) : String(value)
    }

    function descriptionFor(value) {
        return value && typeof value === "object" && value.description !== undefined
            ? String(value.description) : ""
    }

    function detailFor(value) {
        return value && typeof value === "object" && value.detail !== undefined
            ? String(value.detail) : ""
    }

    leftPadding: 0
    rightPadding: 0
    topPadding: 0
    bottomPadding: 0

    implicitWidth: Math.max(280, contentItem.implicitWidth)
    implicitHeight: contentItem.implicitHeight

    contentItem: Column {
        id: column
        spacing: 0

        Repeater {
            id: repeater
            model: root.model

            delegate: Item {
                id: entry
                required property int index
                required property var modelData

                readonly property bool completed: root.currentIndex >= 0 && index < root.currentIndex
                readonly property bool current: index === root.currentIndex

                width: column.width
                implicitHeight: Math.max(Units.indicatorSize, textColumn.implicitHeight) + Units.md
                height: implicitHeight

                RowLayout {
                    anchors.fill: parent
                    spacing: Units.md

                    Item {
                        id: markerColumn
                        Layout.preferredWidth: Units.controlHeight
                        Layout.minimumHeight: Units.indicatorSize
                        Layout.fillHeight: true

                        Rectangle {
                            id: line
                            anchors.horizontalCenter: marker.horizontalCenter
                            anchors.top: marker.verticalCenter
                            anchors.bottom: parent.bottom
                            visible: entry.index < repeater.count - 1
                            width: 2
                            color: entry.completed ? Tokens.accent : Tokens.border
                        }

                        Rectangle {
                            id: marker
                            anchors.top: parent.top
                            anchors.horizontalCenter: parent.horizontalCenter
                            width: Units.indicatorSize
                            height: width
                            radius: width / 2
                            color: entry.completed || entry.current ? Tokens.accent : Tokens.surface
                            border.width: 2
                            border.color: entry.completed || entry.current ? Tokens.accent : Tokens.borderStrong

                            Rectangle {
                                anchors.centerIn: parent
                                visible: entry.current
                                width: Math.max(4, parent.width / 3)
                                height: width
                                radius: width / 2
                                color: Tokens.onAccent
                            }
                        }
                    }

                    ColumnLayout {
                        id: textColumn
                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignTop
                        spacing: Units.xs

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: Units.sm

                            Text {
                                Layout.fillWidth: true
                                text: root.titleFor(entry.modelData)
                                color: entry.current ? Tokens.textPrimary : Tokens.textSecondary
                                font.pixelSize: Typography.body
                                font.weight: entry.current ? Typography.weightSemibold : Typography.weightMedium
                                wrapMode: Text.Wrap
                            }

                            Text {
                                visible: root.detailFor(entry.modelData).length > 0
                                text: root.detailFor(entry.modelData)
                                color: Tokens.textSecondary
                                font.pixelSize: Typography.caption
                            }
                        }

                        Text {
                            Layout.fillWidth: true
                            visible: root.descriptionFor(entry.modelData).length > 0
                            text: root.descriptionFor(entry.modelData)
                            color: Tokens.textSecondary
                            font.pixelSize: Typography.caption
                            wrapMode: Text.Wrap
                        }
                    }
                }
            }
        }
    }

    background: Item { }
}
