import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts

Controls.Control {
    id: root

    property string text: ""
    property int tone: LfBadge.Neutral
    property bool closable: false
    property bool selected: false

    signal closeRequested()
    signal clicked()

    readonly property color toneColor: tone === LfBadge.Accent ? Tokens.accent
        : tone === LfBadge.Info ? Tokens.info
        : tone === LfBadge.Success ? Tokens.success
        : tone === LfBadge.Warning ? Tokens.warning
        : tone === LfBadge.Error ? Tokens.error
        : tone === LfBadge.Critical ? Tokens.critical
        : Tokens.textSecondary

    implicitWidth: contentItem.implicitWidth + leftPadding + rightPadding
    implicitHeight: Math.max(24, contentItem.implicitHeight + topPadding + bottomPadding)
    leftPadding: Units.sm
    rightPadding: Units.xs
    topPadding: Units.xs
    bottomPadding: Units.xs

    Accessible.name: text
    Accessible.role: Accessible.Button

    TapHandler {
        onTapped: root.clicked()
    }

    contentItem: RowLayout {
        spacing: Units.xs

        Text {
            Layout.alignment: Qt.AlignVCenter
            text: root.text
            color: root.toneColor
            font.pixelSize: Typography.caption
            font.weight: root.selected ? Typography.weightSemibold : Typography.weightMedium
        }

        LfToolButton {
            Layout.alignment: Qt.AlignVCenter
            visible: root.closable
            compact: true
            implicitWidth: 20
            implicitHeight: 20
            text: "×"
            Accessible.name: qsTr("Remove %1").arg(root.text)
            onClicked: root.closeRequested()
        }
    }

    background: Rectangle {
        radius: Units.radiusRound
        color: root.selected
            ? Tokens.selection
            : (Theme.highContrast ? "transparent"
                                  : Qt.rgba(root.toneColor.r, root.toneColor.g, root.toneColor.b,
                                            Theme.dark ? 0.16 : 0.08))
        border.width: 1
        border.color: root.selected ? root.toneColor : Tokens.border
    }
}
