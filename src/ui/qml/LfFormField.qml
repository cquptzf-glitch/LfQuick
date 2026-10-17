import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts

Controls.Control {
    id: root

    property string label: ""
    property string description: ""
    property string errorText: ""
    property bool required: false
    property string requiredMarker: "*"

    default property alias contentData: fieldHost.data

    readonly property bool hasError: errorText.length > 0

    leftPadding: 0
    rightPadding: 0
    topPadding: 0
    bottomPadding: 0

    implicitWidth: Math.max(240, contentItem.implicitWidth)
    implicitHeight: contentItem.implicitHeight

    Accessible.name: label
    Accessible.description: hasError ? errorText : description

    contentItem: ColumnLayout {
        spacing: Units.xs

        RowLayout {
            Layout.fillWidth: true
            visible: root.label.length > 0
            spacing: Units.xs

            Text {
                Layout.fillWidth: true
                text: root.label
                color: root.enabled ? Tokens.textPrimary : Tokens.textDisabled
                font.pixelSize: Typography.body
                font.weight: Typography.weightMedium
                wrapMode: Text.Wrap
            }

            Text {
                visible: root.required
                text: root.requiredMarker
                color: Tokens.error
                font.pixelSize: Typography.body
                font.weight: Typography.weightSemibold
            }
        }

        Text {
            Layout.fillWidth: true
            visible: root.description.length > 0
            text: root.description
            color: root.enabled ? Tokens.textSecondary : Tokens.textDisabled
            font.pixelSize: Typography.caption
            wrapMode: Text.Wrap
        }

        ColumnLayout {
            id: fieldHost
            Layout.fillWidth: true
            spacing: Units.sm
            enabled: root.enabled
        }

        Text {
            Layout.fillWidth: true
            visible: root.hasError
            text: root.errorText
            color: Tokens.error
            font.pixelSize: Typography.caption
            wrapMode: Text.Wrap
        }
    }

    background: Item { }
}
