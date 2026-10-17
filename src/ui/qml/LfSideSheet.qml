import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts

Controls.Popup {
    id: root

    property string title: ""
    property int edge: Qt.RightEdge
    property int preferredWidth: Theme.density === Theme.Touch ? 380 : 340
    property bool showCloseButton: true

    default property alias contentData: body.data

    modal: true
    focus: true
    padding: 0
    closePolicy: Controls.Popup.CloseOnEscape | Controls.Popup.CloseOnPressOutside

    width: Math.min(preferredWidth, parent ? parent.width : preferredWidth)
    height: parent ? parent.height : implicitHeight
    x: edge === Qt.LeftEdge ? 0 : Math.max(0, (parent ? parent.width : width) - width)
    y: 0

    enter: Transition {
        NumberAnimation {
            property: "opacity"
            from: 0
            to: 1
            duration: Theme.animationEnabled ? Tokens.animationFast : 0
        }
    }

    exit: Transition {
        NumberAnimation {
            property: "opacity"
            from: 1
            to: 0
            duration: Theme.animationEnabled ? Tokens.animationFast : 0
        }
    }

    contentItem: ColumnLayout {
        spacing: 0

        RowLayout {
            Layout.fillWidth: true
            Layout.leftMargin: Units.lg
            Layout.rightMargin: Units.sm
            Layout.topMargin: Units.sm
            Layout.bottomMargin: Units.sm
            spacing: Units.md

            Text {
                Layout.fillWidth: true
                text: root.title
                color: Tokens.textPrimary
                font.pixelSize: Typography.subtitle
                font.weight: Typography.weightSemibold
                elide: Text.ElideRight
            }

            LfToolButton {
                visible: root.showCloseButton
                compact: true
                text: qsTr("×")
                Accessible.name: qsTr("Close")
                onClicked: root.close()
            }
        }

        LfDivider {
            Layout.fillWidth: true
        }

        ColumnLayout {
            id: body
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.margins: Units.lg
            spacing: Units.md
        }
    }

    background: Rectangle {
        color: Tokens.surface
        border.width: 1
        border.color: Tokens.border
    }
}
