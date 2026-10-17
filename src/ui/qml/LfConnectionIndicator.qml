import QtQuick

Item {
    id: root

    enum State { Disconnected, Connecting, Connected, Warning, Error }

    property int state: LfConnectionIndicator.Disconnected
    property string text: ""
    property string detail: ""
    property bool pulseWhileConnecting: true
    property int indicatorSize: Tokens.indicatorSize

    readonly property color stateColor: state === LfConnectionIndicator.Connected ? Tokens.success
        : state === LfConnectionIndicator.Connecting ? Tokens.info
        : state === LfConnectionIndicator.Warning ? Tokens.warning
        : state === LfConnectionIndicator.Error ? Tokens.error
        : Tokens.textDisabled

    readonly property string defaultText: state === LfConnectionIndicator.Connected ? qsTr("Connected")
        : state === LfConnectionIndicator.Connecting ? qsTr("Connecting")
        : state === LfConnectionIndicator.Warning ? qsTr("Connection warning")
        : state === LfConnectionIndicator.Error ? qsTr("Connection error")
        : qsTr("Disconnected")

    implicitWidth: row.implicitWidth
    implicitHeight: Math.max(Tokens.controlHeight, row.implicitHeight)
    Accessible.role: Accessible.Indicator
    Accessible.name: text.length > 0 ? text : defaultText

    Row {
        id: row
        anchors.verticalCenter: parent.verticalCenter
        spacing: Tokens.controlSpacing

        Rectangle {
            id: indicator
            anchors.verticalCenter: parent.verticalCenter
            width: root.indicatorSize
            height: width
            radius: width / 2
            color: root.stateColor
            border.width: 1
            border.color: Tokens.borderStrong

            SequentialAnimation on opacity {
                running: root.state === LfConnectionIndicator.Connecting
                         && root.pulseWhileConnecting
                         && Theme.animationEnabled
                loops: Animation.Infinite
                NumberAnimation { from: 1.0; to: 0.35; duration: 450; easing.type: Easing.InOutSine }
                NumberAnimation { from: 0.35; to: 1.0; duration: 450; easing.type: Easing.InOutSine }
            }
        }

        Column {
            anchors.verticalCenter: parent.verticalCenter
            spacing: 1

            Text {
                text: root.text.length > 0 ? root.text : root.defaultText
                color: Tokens.textPrimary
                font.pixelSize: Tokens.fontSizeBody
            }

            Text {
                visible: root.detail.length > 0
                text: root.detail
                color: Tokens.textSecondary
                font.pixelSize: Tokens.fontSizeCaption
            }
        }
    }
}
