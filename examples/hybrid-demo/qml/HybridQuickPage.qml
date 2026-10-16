import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import LfQuick

LfPage {
    id: root
    title: qsTr("Hybrid Quick Page")

    Rectangle {
        anchors.fill: parent
        color: Tokens.surface

        ColumnLayout {
            anchors.centerIn: parent
            width: Math.min(parent.width - 64, 640)
            spacing: Tokens.spacingMedium

            Label {
                Layout.fillWidth: true
                text: qsTr("QML page inside QWidget shell")
                font.pixelSize: 28
                font.bold: true
                color: Tokens.textPrimary
                horizontalAlignment: Text.AlignHCenter
            }

            Label {
                Layout.fillWidth: true
                text: qsTr("This page is created by QuickPageProvider, while the legacy page is created by WidgetPageProvider. Both share the same PageRouter history and lifecycle.")
                wrapMode: Text.WordWrap
                color: Tokens.textSecondary
                horizontalAlignment: Text.AlignHCenter
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 96
                radius: Tokens.radiusMedium
                color: Tokens.surfaceSecondary
                border.color: Tokens.border

                RowLayout {
                    anchors.fill: parent
                    anchors.margins: Tokens.spacingMedium

                    Label {
                        text: qsTr("Theme token")
                        color: Tokens.textPrimary
                    }
                    Item { Layout.fillWidth: true }
                    Rectangle {
                        width: 120
                        height: 40
                        radius: Tokens.radiusSmall
                        color: Tokens.accent
                    }
                }
            }
        }
    }
}
