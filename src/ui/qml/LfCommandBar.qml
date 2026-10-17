import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts

Controls.Control {
    id: root

    property bool framed: true
    property bool compact: false
    property int spacing: compact ? Units.xs : Units.sm

    default property alias contentData: actions.data

    implicitWidth: Math.max(160, contentItem.implicitWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(Tokens.controlHeight, contentItem.implicitHeight + topPadding + bottomPadding)
    leftPadding: compact ? Units.sm : Units.md
    rightPadding: leftPadding
    topPadding: compact ? Units.xs : Units.sm
    bottomPadding: topPadding

    contentItem: RowLayout {
        id: actions
        spacing: root.spacing
    }

    background: Rectangle {
        radius: Tokens.radiusMedium
        color: root.framed ? Tokens.surfaceSecondary : "transparent"
        border.width: root.framed ? 1 : 0
        border.color: Tokens.border
    }
}
