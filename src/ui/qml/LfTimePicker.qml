import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts

Controls.Control {
    id: control

    property date value: new Date()
    property int minuteStep: 1
    property string displayFormat: "HH:mm"
    property bool editable: true

    signal accepted(date value)

    implicitWidth: 150
    implicitHeight: Tokens.controlHeight

    function normalizedMinute(value) {
        const step = Math.max(1, Math.min(60, minuteStep))
        const clamped = Math.max(0, Math.min(59, Number(value)))
        return Math.floor(clamped / step) * step
    }

    function applyTime(hour, minute, emitAccepted) {
        const updated = new Date(value.getTime())
        updated.setHours(Math.max(0, Math.min(23, hour)), normalizedMinute(minute), 0, 0)
        value = updated
        if (emitAccepted)
            accepted(value)
    }

    contentItem: LfTextField {
        readOnly: true
        text: Qt.formatTime(control.value, control.displayFormat)
        placeholderText: qsTr("Select time")
        rightPadding: Tokens.controlHeight

        TapHandler {
            enabled: control.enabled && control.editable
            onTapped: timePopup.open()
        }

        Keys.onSpacePressed: function(event) {
            if (control.enabled && control.editable) {
                timePopup.open()
                event.accepted = true
            }
        }
        Keys.onReturnPressed: function(event) {
            if (control.enabled && control.editable) {
                timePopup.open()
                event.accepted = true
            }
        }

        Text {
            anchors.right: parent.right
            anchors.rightMargin: Tokens.controlPaddingHorizontal
            anchors.verticalCenter: parent.verticalCenter
            text: "▾"
            color: control.enabled ? Tokens.textSecondary : Tokens.textDisabled
            font.pixelSize: Tokens.fontSizeBody
        }
    }

    background: null

    Controls.Popup {
        id: timePopup
        padding: Tokens.spacingMedium
        modal: false
        focus: true
        closePolicy: Controls.Popup.CloseOnEscape | Controls.Popup.CloseOnPressOutside

        onOpened: {
            hourInput.value = control.value.getHours()
            minuteInput.value = control.normalizedMinute(control.value.getMinutes())
        }

        background: Rectangle {
            radius: Tokens.radiusMedium
            color: Tokens.surface
            border.color: Tokens.border
        }

        contentItem: ColumnLayout {
            spacing: Tokens.spacingMedium

            RowLayout {
                spacing: Tokens.spacingSmall

                LfNumberInput {
                    id: hourInput
                    from: 0
                    to: 23
                    editable: true
                    Layout.preferredWidth: 84
                }

                Text {
                    text: ":"
                    color: Tokens.textPrimary
                    font.pixelSize: Tokens.fontSizeBody + 2
                }

                LfNumberInput {
                    id: minuteInput
                    from: 0
                    to: 59
                    stepSize: Math.max(1, control.minuteStep)
                    editable: true
                    Layout.preferredWidth: 84
                }
            }

            RowLayout {
                Layout.fillWidth: true
                Item { Layout.fillWidth: true }
                LfButton {
                    text: qsTr("Cancel")
                    onClicked: timePopup.close()
                }
                LfButton {
                    text: qsTr("OK")
                    highlighted: true
                    onClicked: {
                        control.applyTime(hourInput.value, minuteInput.value, true)
                        timePopup.close()
                    }
                }
            }
        }
    }
}
