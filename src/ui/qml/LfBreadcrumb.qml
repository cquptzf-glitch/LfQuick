import QtQuick
import QtQuick.Controls as Controls

Controls.Control {
    id: root

    property var model: []
    property string separator: "›"
    property bool currentEnabled: false

    signal activated(int index, var value)

    implicitWidth: row.implicitWidth + leftPadding + rightPadding
    implicitHeight: Math.max(Tokens.controlHeight, row.implicitHeight + topPadding + bottomPadding)
    leftPadding: Units.xs
    rightPadding: Units.xs
    topPadding: 0
    bottomPadding: 0

    contentItem: Row {
        id: row
        spacing: Units.xs

        Repeater {
            id: repeater
            model: root.model

            delegate: Row {
                required property int index
                required property var modelData
                spacing: Units.xs

                LfToolButton {
                    compact: false
                    text: typeof parent.modelData === "string"
                          ? parent.modelData
                          : (parent.modelData && parent.modelData.text !== undefined
                             ? String(parent.modelData.text) : String(parent.modelData))
                    enabled: parent.index < repeater.count - 1 || root.currentEnabled
                    font.weight: parent.index === repeater.count - 1
                               ? Typography.weightSemibold
                               : Typography.weightRegular
                    onClicked: root.activated(parent.index, parent.modelData)
                }

                Text {
                    anchors.verticalCenter: parent.verticalCenter
                    visible: parent.index < repeater.count - 1
                    text: root.separator
                    color: Tokens.textSecondary
                    font.pixelSize: Typography.body
                }
            }
        }
    }

    background: Item { }
}
