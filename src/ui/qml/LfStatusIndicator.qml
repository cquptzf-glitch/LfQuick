import QtQuick

Item {
    id: control

    enum Status {
        Neutral,
        Info,
        Success,
        Warning,
        Error,
        Critical
    }

    property int status: LfStatusIndicator.Neutral
    property string text: ""
    property int indicatorSize: Math.max(8, Tokens.indicatorSize / 2)
    property color customColor: "transparent"

    readonly property color statusColor: customColor.a > 0 ? customColor
        : status === LfStatusIndicator.Info ? Tokens.info
        : status === LfStatusIndicator.Success ? Tokens.success
        : status === LfStatusIndicator.Warning ? Tokens.warning
        : status === LfStatusIndicator.Error ? Tokens.error
        : status === LfStatusIndicator.Critical ? Tokens.critical
        : Tokens.textSecondary

    implicitWidth: row.implicitWidth
    implicitHeight: Math.max(Tokens.controlHeight, row.implicitHeight)
    Accessible.name: text
    Accessible.role: Accessible.StaticText

    Row {
        id: row
        anchors.verticalCenter: parent.verticalCenter
        spacing: Tokens.controlSpacing

        Rectangle {
            anchors.verticalCenter: parent.verticalCenter
            width: control.indicatorSize
            height: width
            radius: width / 2
            color: control.statusColor
        }

        Text {
            anchors.verticalCenter: parent.verticalCenter
            text: control.text
            color: Tokens.textPrimary
            font.pixelSize: Tokens.fontSizeBody
        }
    }
}
