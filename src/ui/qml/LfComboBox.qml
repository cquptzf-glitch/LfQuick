import QtQuick
import QtQuick.Controls as Controls

Controls.ComboBox {
    id: control

    implicitHeight: Tokens.controlHeight
    leftPadding: Tokens.controlPaddingHorizontal
    rightPadding: Tokens.controlPaddingHorizontal + 18
    topPadding: 0
    bottomPadding: 0
    hoverEnabled: true
    font.pixelSize: Tokens.fontSizeBody

    contentItem: Text {
        leftPadding: 0
        rightPadding: 0
        text: control.displayText
        font: control.font
        color: control.enabled ? Tokens.textPrimary : Tokens.textDisabled
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
    }

    indicator: Text {
        x: control.width - width - Tokens.controlPaddingHorizontal
        y: (control.height - height) / 2
        text: "⌄"
        color: control.enabled ? Tokens.textSecondary : Tokens.textDisabled
        font.pixelSize: Tokens.fontSizeBody + 2
    }

    delegate: Controls.ItemDelegate {
        id: itemDelegate
        required property int index

        width: ListView.view ? ListView.view.width : control.width
        implicitHeight: Tokens.controlHeight
        highlighted: control.highlightedIndex === index
        hoverEnabled: true
        leftPadding: Tokens.controlPaddingHorizontal
        rightPadding: Tokens.controlPaddingHorizontal

        contentItem: Text {
            text: control.textAt(itemDelegate.index)
            color: itemDelegate.enabled ? Tokens.textPrimary : Tokens.textDisabled
            font: control.font
            verticalAlignment: Text.AlignVCenter
            elide: Text.ElideRight
        }

        background: Rectangle {
            radius: Tokens.radiusSmall
            color: itemDelegate.highlighted || itemDelegate.hovered
                   ? Tokens.controlFillHover
                   : "transparent"
        }
    }

    popup: Controls.Popup {
        y: control.height + 2
        width: control.width
        implicitHeight: Math.min(contentItem.implicitHeight + topPadding + bottomPadding, 320)
        topPadding: 4
        bottomPadding: 4
        leftPadding: 4
        rightPadding: 4
        closePolicy: Controls.Popup.CloseOnEscape | Controls.Popup.CloseOnPressOutsideParent

        contentItem: ListView {
            clip: true
            implicitHeight: contentHeight
            model: control.popup.visible ? control.delegateModel : null
            currentIndex: control.highlightedIndex
            boundsBehavior: Flickable.StopAtBounds

            Controls.ScrollIndicator.vertical: Controls.ScrollIndicator {}
        }

        background: Rectangle {
            radius: Tokens.radiusMedium
            color: Tokens.surface
            border.color: Tokens.border
        }
    }

    background: Rectangle {
        radius: Tokens.radiusSmall
        color: !control.enabled ? Tokens.controlFillDisabled
             : control.down ? Tokens.controlFillPressed
             : control.hovered ? Tokens.controlFillHover
             : Tokens.controlFill
        border.width: control.activeFocus ? 2 : 1
        border.color: control.activeFocus ? Tokens.focusRing : Tokens.border
    }
}
