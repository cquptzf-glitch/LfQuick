import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts

Controls.Control {
    id: control

    property date startDate: new Date()
    property date endDate: new Date()
    property date minimumDate: new Date(1900, 0, 1)
    property date maximumDate: new Date(2100, 11, 31)
    property bool editable: true
    readonly property bool valid: startDate.getTime() <= endDate.getTime()

    signal rangeAccepted(date startDate, date endDate)

    implicitWidth: 380
    implicitHeight: Tokens.controlHeight

    function acceptStart(dateValue) {
        startDate = dateValue
        if (startDate.getTime() > endDate.getTime())
            endDate = new Date(startDate.getTime())
        rangeAccepted(startDate, endDate)
    }

    function acceptEnd(dateValue) {
        endDate = dateValue
        if (endDate.getTime() < startDate.getTime())
            startDate = new Date(endDate.getTime())
        rangeAccepted(startDate, endDate)
    }

    contentItem: RowLayout {
        spacing: Tokens.spacingSmall

        LfDatePicker {
            Layout.fillWidth: true
            value: control.startDate
            minimumDate: control.minimumDate
            maximumDate: control.endDate.getTime() < control.maximumDate.getTime()
                         ? control.endDate : control.maximumDate
            editable: control.editable
            onAccepted: function(dateValue) { control.acceptStart(dateValue) }
        }

        Text {
            text: "—"
            color: Tokens.textSecondary
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }

        LfDatePicker {
            Layout.fillWidth: true
            value: control.endDate
            minimumDate: control.startDate.getTime() > control.minimumDate.getTime()
                         ? control.startDate : control.minimumDate
            maximumDate: control.maximumDate
            editable: control.editable
            onAccepted: function(dateValue) { control.acceptEnd(dateValue) }
        }
    }

    background: null
}
