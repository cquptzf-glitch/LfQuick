import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts

Controls.Control {
    id: control

    property date value: new Date()
    property date minimumDate: new Date(1900, 0, 1)
    property date maximumDate: new Date(2100, 11, 31)
    property string displayFormat: "yyyy-MM-dd"
    property bool editable: true

    signal accepted(date value)

    implicitWidth: 180
    implicitHeight: Tokens.controlHeight

    function dateOnly(dateValue) {
        return new Date(dateValue.getFullYear(), dateValue.getMonth(), dateValue.getDate())
    }

    function inRange(dateValue) {
        const time = dateOnly(dateValue).getTime()
        return time >= dateOnly(minimumDate).getTime()
            && time <= dateOnly(maximumDate).getTime()
    }

    function setValue(dateValue, emitAccepted) {
        if (!dateValue || isNaN(dateValue.getTime()) || !inRange(dateValue))
            return false
        const normalized = dateOnly(dateValue)
        if (value.getTime() !== normalized.getTime())
            value = normalized
        if (emitAccepted)
            accepted(value)
        return true
    }

    contentItem: LfTextField {
        id: displayField
        readOnly: true
        text: Qt.formatDate(control.value, control.displayFormat)
        placeholderText: qsTr("Select date")
        rightPadding: Tokens.controlHeight

        TapHandler {
            enabled: control.enabled && control.editable
            onTapped: calendarPopup.open()
        }

        Keys.onSpacePressed: function(event) {
            if (control.enabled && control.editable) {
                calendarPopup.open()
                event.accepted = true
            }
        }
        Keys.onReturnPressed: function(event) {
            if (control.enabled && control.editable) {
                calendarPopup.open()
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
        id: calendarPopup
        parent: Controls.Overlay.overlay
        width: 310
        height: 330
        padding: Tokens.spacingMedium
        modal: false
        focus: true
        closePolicy: Controls.Popup.CloseOnEscape | Controls.Popup.CloseOnPressOutside

        onOpened: {
            monthGrid.month = control.value.getMonth()
            monthGrid.year = control.value.getFullYear()
        }

        background: Rectangle {
            radius: Tokens.radiusMedium
            color: Tokens.surface
            border.color: Tokens.border
        }

        contentItem: ColumnLayout {
            spacing: Tokens.spacingSmall

            RowLayout {
                Layout.fillWidth: true

                LfButton {
                    text: "‹"
                    Layout.preferredWidth: Tokens.controlHeight
                    onClicked: {
                        if (monthGrid.month === Calendar.January) {
                            monthGrid.month = Calendar.December
                            monthGrid.year -= 1
                        } else {
                            monthGrid.month -= 1
                        }
                    }
                }

                Text {
                    Layout.fillWidth: true
                    text: monthGrid.title
                    horizontalAlignment: Text.AlignHCenter
                    color: Tokens.textPrimary
                    font.pixelSize: Tokens.fontSizeBody
                    font.bold: true
                }

                LfButton {
                    text: "›"
                    Layout.preferredWidth: Tokens.controlHeight
                    onClicked: {
                        if (monthGrid.month === Calendar.December) {
                            monthGrid.month = Calendar.January
                            monthGrid.year += 1
                        } else {
                            monthGrid.month += 1
                        }
                    }
                }
            }

            Controls.DayOfWeekRow {
                Layout.fillWidth: true
                locale: monthGrid.locale
                delegate: Text {
                    required property string shortName
                    text: shortName
                    color: Tokens.textSecondary
                    font.pixelSize: Tokens.fontSizeCaption
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
            }

            Controls.MonthGrid {
                id: monthGrid
                Layout.fillWidth: true
                Layout.fillHeight: true
                month: control.value.getMonth()
                year: control.value.getFullYear()

                delegate: Rectangle {
                    required property var model
                    readonly property bool sameMonth: model.date.getMonth() === monthGrid.month
                    readonly property bool selected: control.dateOnly(model.date).getTime()
                                                     === control.dateOnly(control.value).getTime()
                    readonly property bool selectable: sameMonth && control.inRange(model.date)

                    opacity: sameMonth ? (selectable ? 1.0 : 0.35) : 0.22
                    color: selected ? Tokens.accent : "transparent"
                    radius: Tokens.radiusSmall

                    Text {
                        anchors.centerIn: parent
                        text: model.day
                        color: parent.selected ? Tokens.onAccent : Tokens.textPrimary
                        font.pixelSize: Tokens.fontSizeCaption
                    }
                }

                onClicked: function(dateValue) {
                    if (control.setValue(dateValue, true))
                        calendarPopup.close()
                }
            }
        }
    }
}
