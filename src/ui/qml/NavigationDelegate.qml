import QtQuick
import QtQuick.Layouts

Item {
    id: root

    property string route: ""
    property string label: ""
    property string icon: ""
    property var badge: null
    property bool selected: false
    property bool itemEnabled: true
    property bool compact: false
    property var parameters: ({})

    signal activated(string route, var parameters)

    implicitWidth: 220
    implicitHeight: button.implicitHeight

    LfButton {
        id: button
        anchors.fill: parent
        checkable: true
        checked: root.selected
        highlighted: root.selected
        enabled: root.itemEnabled
        leftPadding: 10
        rightPadding: 10

        contentItem: RowLayout {
            spacing: Tokens.controlSpacing

            Image {
                visible: root.icon.length > 0
                source: root.icon
                sourceSize.width: 18
                sourceSize.height: 18
                Layout.preferredWidth: visible ? 18 : 0
                Layout.preferredHeight: visible ? 18 : 0
                fillMode: Image.PreserveAspectFit
            }

            Text {
                Layout.fillWidth: true
                visible: !root.compact
                text: root.label
                color: button.enabled ? Tokens.textPrimary : Tokens.textSecondary
                font.pixelSize: Tokens.fontSizeBody
                elide: Text.ElideRight
            }

            Text {
                visible: root.compact && root.icon.length === 0
                Layout.fillWidth: true
                text: root.label.length > 0 ? root.label.slice(0, 1) : "•"
                color: button.enabled ? Tokens.textPrimary : Tokens.textSecondary
                font.pixelSize: Tokens.fontSizeBody
                horizontalAlignment: Text.AlignHCenter
            }

            Text {
                visible: root.badge !== undefined && root.badge !== null
                         && String(root.badge).length > 0
                text: String(root.badge)
                color: root.selected ? Tokens.onAccent : Tokens.textSecondary
                font.pixelSize: Tokens.fontSizeCaption
            }
        }

        onClicked: root.activated(root.route, root.parameters || {})
    }
}
