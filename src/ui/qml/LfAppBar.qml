import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts

Controls.Control {
    id: root

    property var controller
    property string title: ""
    property string subtitle: ""
    property url iconSource
    property bool dragEnabled: true
    property bool showMinimizeButton: true
    property bool showMaximizeButton: true
    property bool showCloseButton: true
    property Component leadingContent
    property Component trailingContent

    signal titleDoubleClicked()

    implicitHeight: Theme.density === Theme.Touch ? 52 : 42
    leftPadding: Tokens.spacingMedium
    rightPadding: 0
    topPadding: 0
    bottomPadding: 0

    background: Rectangle {
        color: Tokens.surface
        border.color: Tokens.border
        border.width: 0

        Rectangle {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            height: 1
            color: Tokens.border
        }
    }

    contentItem: Item {
        MouseArea {
            anchors.fill: parent
            acceptedButtons: Qt.LeftButton
            enabled: root.dragEnabled && root.controller
            onPressed: root.controller.startSystemMove()
            onDoubleClicked: {
                root.titleDoubleClicked()
                if (root.controller)
                    root.controller.toggleMaximized()
            }
        }

        RowLayout {
            anchors.fill: parent
            spacing: Tokens.spacingSmall

            Loader {
                Layout.alignment: Qt.AlignVCenter
                active: root.leadingContent !== null
                sourceComponent: root.leadingContent
            }

            Image {
                visible: root.iconSource.toString().length > 0
                source: root.iconSource
                sourceSize.width: 20
                sourceSize.height: 20
                Layout.preferredWidth: visible ? 20 : 0
                Layout.preferredHeight: visible ? 20 : 0
                fillMode: Image.PreserveAspectFit
            }

            ColumnLayout {
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignVCenter
                spacing: 0

                Text {
                    Layout.fillWidth: true
                    text: root.title
                    color: Tokens.textPrimary
                    font.pixelSize: Tokens.fontSizeBody
                    font.bold: true
                    elide: Text.ElideRight
                }

                Text {
                    Layout.fillWidth: true
                    visible: root.subtitle.length > 0
                    text: root.subtitle
                    color: Tokens.textSecondary
                    font.pixelSize: Tokens.fontSizeCaption
                    elide: Text.ElideRight
                }
            }

            Loader {
                Layout.alignment: Qt.AlignVCenter
                active: root.trailingContent !== null
                sourceComponent: root.trailingContent
            }

            RowLayout {
                spacing: 0

                component CaptionButton: Controls.ToolButton {
                    id: captionButton
                    implicitWidth: Theme.density === Theme.Touch ? 52 : 46
                    implicitHeight: root.implicitHeight - 1
                    hoverEnabled: true

                    contentItem: Text {
                        text: captionButton.text
                        color: captionButton.enabled ? Tokens.textPrimary : Tokens.textDisabled
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        font.pixelSize: Tokens.fontSizeBody
                    }

                    background: Rectangle {
                        color: captionButton.down ? Tokens.controlFillPressed
                             : captionButton.hovered ? Tokens.controlFillHover
                             : "transparent"
                    }
                }

                CaptionButton {
                    visible: root.showMinimizeButton
                    text: "—"
                    Accessible.name: qsTr("Minimize")
                    onClicked: if (root.controller) root.controller.minimize()
                }

                CaptionButton {
                    visible: root.showMaximizeButton
                    text: root.controller && root.controller.maximized ? "❐" : "□"
                    Accessible.name: root.controller && root.controller.maximized
                                     ? qsTr("Restore") : qsTr("Maximize")
                    onClicked: if (root.controller) root.controller.toggleMaximized()
                }

                CaptionButton {
                    id: closeButton
                    visible: root.showCloseButton
                    text: "×"
                    Accessible.name: qsTr("Close")
                    onClicked: if (root.controller) root.controller.close()

                    background: Rectangle {
                        color: closeButton.down ? Qt.darker(Tokens.error, 1.15)
                             : closeButton.hovered ? Tokens.error
                             : "transparent"
                    }
                    contentItem: Text {
                        text: closeButton.text
                        color: closeButton.hovered || closeButton.down
                               ? Tokens.onAccent : Tokens.textPrimary
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        font.pixelSize: Tokens.fontSizeBody + 2
                    }
                }
            }
        }
    }
}
