import QtQuick
import QtQuick.Controls as Controls

Controls.ComboBox {
    id: root

    implicitHeight: FluentTokens.controlHeight
    leftPadding: FluentTokens.padding
    rightPadding: FluentTokens.padding + 18
    topPadding: 0
    bottomPadding: 0
    hoverEnabled: true
    font.pixelSize: FluentTokens.fontSize

    contentItem: Text {
        text: root.displayText
        font: root.font
        color: root.enabled ? FluentTokens.text : FluentTokens.textSecondary
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
    }

    indicator: Text {
        x: root.width - width - FluentTokens.padding
        y: (root.height - height) / 2
        text: "⌄"
        color: FluentTokens.textSecondary
        font.pixelSize: FluentTokens.fontSize + 2
    }

    background: Rectangle {
        radius: FluentTokens.radius
        color: root.down ? FluentTokens.pressed
             : root.hovered ? FluentTokens.hover
             : FluentTokens.fill
        border.width: root.activeFocus ? 2 : 1
        border.color: root.activeFocus ? FluentTokens.accent : FluentTokens.border
    }

    delegate: Controls.ItemDelegate {
        required property var modelData
        width: root.popup.width - 8
        height: FluentTokens.controlHeight
        text: String(modelData)
        highlighted: root.highlightedIndex === index
        contentItem: Text {
            text: parent.text
            color: FluentTokens.text
            font.pixelSize: FluentTokens.fontSize
            verticalAlignment: Text.AlignVCenter
            elide: Text.ElideRight
        }
        background: Rectangle {
            radius: FluentTokens.radius
            color: parent.highlighted ? FluentTokens.hover : "transparent"
        }
    }

    popup: Controls.Popup {
        y: root.height + 4
        width: root.width
        padding: 4
        implicitHeight: Math.min(contentItem.implicitHeight + topPadding + bottomPadding, 300)
        contentItem: ListView {
            clip: true
            implicitHeight: contentHeight
            model: root.delegateModel
            currentIndex: root.highlightedIndex
        }
        background: Rectangle {
            radius: FluentTokens.radius
            color: FluentTokens.surface
            border.color: FluentTokens.border
        }
    }
}
