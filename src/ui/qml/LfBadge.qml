import QtQuick

Rectangle {
    id: root

    enum Tone {
        Neutral,
        Accent,
        Info,
        Success,
        Warning,
        Error,
        Critical
    }

    property int tone: LfBadge.Neutral
    property string text: ""
    property bool outlined: false
    property int horizontalPadding: Units.sm
    property int verticalPadding: Units.xs

    readonly property color toneColor: tone === LfBadge.Accent ? Tokens.accent
        : tone === LfBadge.Info ? Tokens.info
        : tone === LfBadge.Success ? Tokens.success
        : tone === LfBadge.Warning ? Tokens.warning
        : tone === LfBadge.Error ? Tokens.error
        : tone === LfBadge.Critical ? Tokens.critical
        : Tokens.textSecondary
    readonly property color fillColor: Theme.highContrast
        ? "transparent"
        : Qt.rgba(toneColor.r, toneColor.g, toneColor.b, Theme.dark ? 0.22 : 0.12)

    implicitWidth: Math.max(20, label.implicitWidth + horizontalPadding * 2)
    implicitHeight: Math.max(20, label.implicitHeight + verticalPadding * 2)
    radius: Units.radiusRound
    color: outlined ? "transparent" : fillColor
    border.width: outlined || Theme.highContrast ? 1 : 0
    border.color: toneColor

    Accessible.name: text
    Accessible.role: Accessible.StaticText

    Text {
        id: label
        anchors.centerIn: parent
        text: root.text
        color: root.toneColor
        font.pixelSize: Typography.caption
        font.weight: Typography.weightMedium
    }
}
