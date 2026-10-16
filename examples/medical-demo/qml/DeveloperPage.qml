
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import LfQuick

LfPage {
    title: qsTr("Developer")

    ColumnLayout {
        anchors.centerIn: parent
        spacing: Tokens.spacingMedium
        width: Math.min(parent.width - 48, 720)

        Label {
            text: qsTr("LfQuick Developer Playground")
            font.pixelSize: 28
            font.bold: true
            color: Tokens.textPrimary
        }

        Label {
            text: qsTr("This page exercises runtime Theme/Tokens and is the home for Focus, Overlay, DataGrid and chart stress tools.")
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
            color: Tokens.textSecondary
        }

        RowLayout {
            Label { text: qsTr("Theme"); color: Tokens.textPrimary }
            Button {
                text: Theme.dark ? qsTr("Switch to Light") : qsTr("Switch to Dark")
                onClicked: Theme.dark = !Theme.dark
            }
            Label { text: Theme.dark ? qsTr("Dark") : qsTr("Light"); color: Tokens.textSecondary }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 120
            radius: Tokens.radiusMedium
            color: Tokens.surfaceSecondary
            border.color: Tokens.border

            Column {
                anchors.centerIn: parent
                spacing: Tokens.spacingSmall
                Label { text: qsTr("Semantic token preview"); color: Tokens.textPrimary }
                Row {
                    spacing: Tokens.spacingSmall
                    Repeater {
                        model: [Tokens.accent, Tokens.success, Tokens.warning, Tokens.error, Tokens.critical]
                        Rectangle { required property color modelData; width: 48; height: 24; radius: 4; color: modelData }
                    }
                }
            }
        }
    }
}
