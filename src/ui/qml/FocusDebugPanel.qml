import QtQuick
import QtQuick.Controls
import LfQuick

Frame {
    id: root

    property FocusNavigator navigator

    width: 320
    padding: Tokens.spacingMedium

    Column {
        width: parent.width
        spacing: Tokens.spacingSmall

        Label {
            text: qsTr("Focus chain")
            font.bold: true
        }

        Label {
            width: parent.width
            wrapMode: Text.WordWrap
            text: root.navigator ? root.navigator.focusChain().join(" → ") : qsTr("No scope")
        }

        Row {
            spacing: Tokens.spacingSmall

            Button {
                text: qsTr("Previous")
                onClicked: if (root.navigator) root.navigator.movePrevious()
            }

            Button {
                text: qsTr("Next")
                onClicked: if (root.navigator) root.navigator.moveNext()
            }
        }
    }
}
