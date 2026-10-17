import QtQuick
import QtQuick.Controls as Controls

Controls.Control {
    id: root

    enum State { Empty, Occupied, Running, Success, Warning, Error, Disabled }

    property int state: LfSlotView.Empty
    property string text: ""
    property string detail: ""
    property bool selected: false
    property bool interactive: true
    property color customColor: "transparent"

    readonly property color stateColor: customColor.a > 0 ? customColor
        : state === LfSlotView.Success ? Tokens.success
        : state === LfSlotView.Warning ? Tokens.warning
        : state === LfSlotView.Error ? Tokens.error
        : state === LfSlotView.Running ? Tokens.info
        : state === LfSlotView.Occupied ? Tokens.accent
        : state === LfSlotView.Disabled ? Tokens.textDisabled
        : Tokens.surfaceTertiary

    signal clicked()

    implicitWidth: 64
    implicitHeight: 64
    padding: 4
    Accessible.name: text.length > 0 ? text : qsTr("Slot")
    Accessible.role: Accessible.Button

    background: Rectangle {
        radius: Tokens.radiusMedium
        color: root.stateColor
        opacity: root.state === LfSlotView.Empty ? 0.55 : 1.0
        border.width: root.selected || root.activeFocus ? 3 : 1
        border.color: root.activeFocus ? Tokens.focusRing
                    : root.selected ? Tokens.accent
                    : Tokens.borderStrong
    }

    contentItem: Column {
        anchors.centerIn: parent
        spacing: 1

        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: root.text
            color: root.state === LfSlotView.Empty ? Tokens.textPrimary : Tokens.onAccent
            font.pixelSize: Tokens.fontSizeBody
            font.bold: true
        }

        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            visible: root.detail.length > 0
            text: root.detail
            color: root.state === LfSlotView.Empty ? Tokens.textSecondary : Tokens.onAccent
            opacity: 0.85
            font.pixelSize: Tokens.fontSizeCaption
        }
    }

    TapHandler {
        enabled: root.interactive && root.enabled
        onTapped: root.clicked()
    }
}
