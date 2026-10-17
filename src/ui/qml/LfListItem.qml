import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts

Controls.ItemDelegate {
    id: root

    property string title: ""
    property string subtitle: ""
    property string leadingText: ""
    property string trailingText: ""
    property bool selected: false
    property bool compact: false

    implicitHeight: Math.max(compact ? Tokens.controlHeight : Tokens.controlHeight + Units.md,
                             contentItem.implicitHeight + topPadding + bottomPadding)
    leftPadding: Tokens.controlPaddingHorizontal
    rightPadding: Tokens.controlPaddingHorizontal
    topPadding: compact ? Units.xs : Units.sm
    bottomPadding: topPadding
    hoverEnabled: true
    highlighted: selected

    Accessible.name: title
    Accessible.description: subtitle

    contentItem: RowLayout {
        spacing: Units.md

        Rectangle {
            visible: root.leadingText.length > 0
            width: Units.controlHeight
            height: width
            radius: width / 2
            color: Tokens.surfaceTertiary
            border.width: 1
            border.color: Tokens.border

            Text {
                anchors.centerIn: parent
                text: root.leadingText
                color: Tokens.textSecondary
                font.pixelSize: Typography.caption
                font.weight: Typography.weightSemibold
            }
        }

        ColumnLayout {
            Layout.fillWidth: true
            spacing: Units.xs

            Text {
                Layout.fillWidth: true
                text: root.title
                color: root.enabled ? Tokens.textPrimary : Tokens.textDisabled
                font.pixelSize: Typography.body
                font.weight: Typography.weightMedium
                elide: Text.ElideRight
            }

            Text {
                Layout.fillWidth: true
                visible: root.subtitle.length > 0
                text: root.subtitle
                color: root.enabled ? Tokens.textSecondary : Tokens.textDisabled
                font.pixelSize: Typography.caption
                elide: Text.ElideRight
            }
        }

        Text {
            visible: root.trailingText.length > 0
            text: root.trailingText
            color: root.enabled ? Tokens.textSecondary : Tokens.textDisabled
            font.pixelSize: Typography.body
            horizontalAlignment: Text.AlignRight
        }
    }

    background: Rectangle {
        radius: Tokens.radiusSmall
        color: root.selected ? Tokens.selection
             : root.down ? Tokens.controlFillPressed
             : root.hovered ? Tokens.controlFillHover
             : "transparent"
        border.width: root.activeFocus ? 2 : 0
        border.color: Tokens.focusRing
    }
}
