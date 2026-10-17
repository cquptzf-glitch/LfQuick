import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts

Controls.Control {
    id: root

    enum Appearance {
        Filled,
        Outlined
    }

    property int appearance: LfCard.Filled
    property string title: ""
    property string subtitle: ""

    default property alias contentData: body.data

    leftPadding: Units.lg
    rightPadding: Units.lg
    topPadding: Units.lg
    bottomPadding: Units.lg

    implicitWidth: Math.max(180, contentItem.implicitWidth + leftPadding + rightPadding)
    implicitHeight: contentItem.implicitHeight + topPadding + bottomPadding

    contentItem: ColumnLayout {
        id: body
        spacing: Units.md

        Text {
            Layout.fillWidth: true
            visible: root.title.length > 0
            text: root.title
            color: Tokens.textPrimary
            font.pixelSize: Typography.subtitle
            font.weight: Typography.weightSemibold
            wrapMode: Text.Wrap
        }

        Text {
            Layout.fillWidth: true
            visible: root.subtitle.length > 0
            text: root.subtitle
            color: Tokens.textSecondary
            font.pixelSize: Typography.body
            wrapMode: Text.Wrap
        }
    }

    background: Rectangle {
        radius: Units.radiusMedium
        color: root.appearance === LfCard.Filled ? Tokens.surfaceSecondary : Tokens.surface
        border.width: 1
        border.color: root.appearance === LfCard.Outlined ? Tokens.borderStrong : Tokens.border
    }
}
