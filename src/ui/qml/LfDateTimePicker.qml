import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts

Controls.Control {
    id: control

    property date value: new Date()
    property date minimumDate: new Date(1900, 0, 1)
    property date maximumDate: new Date(2100, 11, 31)
    property int minuteStep: 1
    property bool editable: true

    signal accepted(date value)

    implicitWidth: 340
    implicitHeight: Tokens.controlHeight

    function combine(datePart, timePart) {
        return new Date(datePart.getFullYear(),
                        datePart.getMonth(),
                        datePart.getDate(),
                        timePart.getHours(),
                        timePart.getMinutes(),
                        0,
                        0)
    }

    contentItem: RowLayout {
        spacing: Tokens.spacingSmall

        LfDatePicker {
            id: datePicker
            Layout.fillWidth: true
            value: control.value
            minimumDate: control.minimumDate
            maximumDate: control.maximumDate
            editable: control.editable
            onAccepted: function(dateValue) {
                control.value = control.combine(dateValue, control.value)
                control.accepted(control.value)
            }
        }

        LfTimePicker {
            id: timePicker
            Layout.preferredWidth: 140
            value: control.value
            minuteStep: control.minuteStep
            editable: control.editable
            onAccepted: function(timeValue) {
                control.value = control.combine(control.value, timeValue)
                control.accepted(control.value)
            }
        }
    }

    background: null
}
