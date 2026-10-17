import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts

Controls.Control {
    id: root

    property string title: ""
    property string description: ""
    property bool expanded: false
    property bool headerEnabled: true

    default property alias contentData: body.data

    signal toggled(bool expanded)

    function toggle() {
        if (!headerEnabled)
            return
        expanded = !expanded
        toggled(expanded)
    }

    leftPadding: 0
    rightPadding: 0
    topPadding: 0
    bottomPadding: 0

    implicitWidth: Math.max(180, contentItem.implicitWidth)
    implicitHeight: contentItem.implicitHeight

    contentItem: ColumnLayout {
        spacing: Units.sm

        LfToolButton {
            Layout.fillWidth: true
            enabled: root.headerEnabled
            text: (root.expanded ? "⌄  " : "›  ") + root.title
            font.weight: Typography.weightSemibold
            onClicked: root.toggle()
        }

        Text {
            Layout.fillWidth: true
            Layout.leftMargin: Tokens.controlPaddingHorizontal
            Layout.rightMargin: Tokens.controlPaddingHorizontal
            visible: root.description.length > 0
            text: root.description
            color: Tokens.textSecondary
            font.pixelSize: Typography.caption
            wrapMode: Text.Wrap
        }

        Item {
            id: contentClip
            Layout.fillWidth: true
            Layout.preferredHeight: implicitHeight
            implicitHeight: root.expanded ? body.implicitHeight : 0
            clip: true
            visible: implicitHeight > 0

            Behavior on implicitHeight {
                NumberAnimation {
                    duration: Theme.animationEnabled ? Tokens.animationNormal : 0
                    easing.type: Easing.OutCubic
                }
            }

            ColumnLayout {
                id: body
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: parent.top
                spacing: Units.sm
            }
        }
    }

    background: Rectangle {
        radius: Tokens.radiusMedium
        color: Tokens.surfaceSecondary
        border.width: 1
        border.color: Tokens.border
    }
}
