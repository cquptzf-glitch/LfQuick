import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts

Controls.Control {
    id: root

    enum Severity {
        Info,
        Success,
        Warning,
        Error,
        Critical
    }

    property int severity: LfInfoBar.Info
    property string title: ""
    property string message: ""
    property bool closable: false
    property bool autoHideOnClose: true
    property bool closed: false

    signal closeRequested()

    readonly property color severityColor: severity === LfInfoBar.Success ? Tokens.success
        : severity === LfInfoBar.Warning ? Tokens.warning
        : severity === LfInfoBar.Error ? Tokens.error
        : severity === LfInfoBar.Critical ? Tokens.critical
        : Tokens.info

    visible: !closed
    leftPadding: Units.md
    rightPadding: Units.md
    topPadding: Units.md
    bottomPadding: Units.md

    implicitWidth: Math.max(240, contentItem.implicitWidth + leftPadding + rightPadding)
    implicitHeight: contentItem.implicitHeight + topPadding + bottomPadding

    function close() {
        closeRequested()
        if (autoHideOnClose)
            closed = true
    }

    function requestClose() {
        close()
    }

    contentItem: RowLayout {
        spacing: Units.md

        Rectangle {
            Layout.alignment: Qt.AlignTop
            Layout.topMargin: 4
            width: Math.max(8, Units.indicatorSize / 2)
            height: width
            radius: width / 2
            color: root.severityColor
        }

        ColumnLayout {
            Layout.fillWidth: true
            spacing: Units.xs

            Text {
                Layout.fillWidth: true
                visible: root.title.length > 0
                text: root.title
                color: Tokens.textPrimary
                font.pixelSize: Typography.body
                font.weight: Typography.weightSemibold
                wrapMode: Text.Wrap
            }

            Text {
                Layout.fillWidth: true
                visible: root.message.length > 0
                text: root.message
                color: Tokens.textSecondary
                font.pixelSize: Typography.body
                wrapMode: Text.Wrap
            }
        }

        LfToolButton {
            Layout.alignment: Qt.AlignTop
            visible: root.closable
            compact: true
            text: qsTr("×")
            Accessible.name: qsTr("Close")
            onClicked: root.close()
        }
    }

    background: Rectangle {
        radius: Tokens.radiusMedium
        color: Theme.highContrast
            ? Tokens.surface
            : Qt.rgba(root.severityColor.r, root.severityColor.g, root.severityColor.b,
                      Theme.dark ? 0.16 : 0.08)
        border.width: 1
        border.color: root.severityColor
    }
}
