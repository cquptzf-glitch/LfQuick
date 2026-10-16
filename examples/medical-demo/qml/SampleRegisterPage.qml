import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import LfQuick

LfPage {
    id: root
    title: qsTr("Sample Register")

    property bool pageActive: false
    property bool dirty: false
    property bool submitting: false
    property bool suppressDirty: false
    property bool scanArmed: true
    property string busyToken: ""
    property string statusMessage: qsTr("Scan or enter a sample, then submit.")
    property string barcodeError: ""
    property string sampleIdError: ""
    property string rackError: ""

    readonly property var barcodeRule: ({
        "id": "sample.barcode", "required": true,
        "minLength": 4, "maxLength": 32,
        "regularExpression": "[A-Za-z0-9_-]+"
    })
    readonly property var sampleIdRule: ({
        "id": "sample.id", "required": true,
        "minLength": 3, "maxLength": 24,
        "regularExpression": "[A-Za-z0-9_-]+"
    })
    readonly property var rackRule: ({
        "id": "sample.rack", "required": true,
        "minLength": 1, "maxLength": 12,
        "regularExpression": "[A-Za-z0-9_-]+"
    })

    ValidationEvaluator { id: validation }
    FocusNavigator { id: focusNavigator; scope: formCard }

    function markDirty() {
        if (root.suppressDirty || !root.pageActive || root.submitting)
            return
        root.dirty = true
        root.updateGuard()
    }

    function updateGuard() {
        var state = quickRuntime.navigationGuard.state
        if (!root.pageActive) {
            state.dirty = false
            state.busy = false
            state.reason = ""
            state.confirmRequired = true
            return
        }
        state.dirty = root.dirty && !root.submitting
        state.busy = root.submitting
        state.confirmRequired = !root.submitting
        state.reason = root.submitting
                ? qsTr("Sample submission is in progress")
                : (root.dirty ? qsTr("Sample registration contains unsaved changes") : "")
    }

    function updateScanner() {
        scanner.enabled = root.pageActive
                && root.scanArmed
                && barcodeField.activeFocus
                && !root.submitting
    }

    function validateForm() {
        var barcodeResult = validation.validate(barcodeField.text.trim(), root.barcodeRule)
        var idResult = validation.validate(sampleIdField.text.trim(), root.sampleIdRule)
        var rackResult = validation.validate(rackField.text.trim(), root.rackRule)

        root.barcodeError = barcodeResult.valid ? "" : barcodeResult.message
        root.sampleIdError = idResult.valid ? "" : idResult.message
        root.rackError = rackResult.valid ? "" : rackResult.message

        if (barcodeResult.valid && sampleModel.findFirst("barcode", barcodeField.text.trim(), Qt.CaseInsensitive) >= 0)
            root.barcodeError = qsTr("Barcode already exists")
        if (idResult.valid && sampleModel.findFirst("sampleId", sampleIdField.text.trim(), Qt.CaseInsensitive) >= 0)
            root.sampleIdError = qsTr("Sample ID already exists")

        var valid = root.barcodeError.length === 0
                && root.sampleIdError.length === 0
                && root.rackError.length === 0
        if (!valid)
            root.statusMessage = qsTr("Please correct the highlighted fields before submitting.")
        return valid
    }

    function resetForm() {
        root.suppressDirty = true
        barcodeField.text = ""
        sampleIdField.text = ""
        sampleTypeBox.currentIndex = 0
        rackField.text = "R1"
        positionBox.value = 1
        priorityBox.currentIndex = 0
        root.barcodeError = ""
        root.sampleIdError = ""
        root.rackError = ""
        root.suppressDirty = false
        root.dirty = false
        root.updateGuard()
        barcodeField.forceActiveFocus()
        root.updateScanner()
    }

    function submit() {
        if (root.submitting || !root.validateForm())
            return
        root.submitting = true
        root.updateScanner()
        root.updateGuard()
        root.busyToken = overlayCoordinator.beginBusy(qsTr("Registering sample..."), false)
        submitTimer.restart()
    }

    onActivated: {
        root.pageActive = true
        root.updateGuard()
        Qt.callLater(function() {
            barcodeField.forceActiveFocus()
            root.updateScanner()
        })
    }

    onDeactivated: {
        root.pageActive = false
        scanner.enabled = false
        root.updateGuard()
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: Tokens.spacingLarge
        spacing: Tokens.spacingMedium

        RowLayout {
            Layout.fillWidth: true
            Label {
                text: qsTr("Sample Register")
                font.pixelSize: 26
                font.bold: true
                color: Tokens.textPrimary
            }
            Item { Layout.fillWidth: true }
            Switch {
                text: qsTr("Scanner armed")
                checked: root.scanArmed
                enabled: !root.submitting
                onToggled: {
                    root.scanArmed = checked
                    root.updateScanner()
                }
            }
            Button {
                text: qsTr("Simulate scan")
                enabled: !root.submitting
                onClicked: scanner.simulateScan("REG-" + (sampleModel.count + 1))
            }
        }

        Rectangle {
            id: formCard
            Layout.fillWidth: true
            Layout.fillHeight: true
            radius: Tokens.radiusMedium
            color: Tokens.surfaceSecondary
            border.color: Tokens.border

            GridLayout {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: parent.top
                anchors.margins: Tokens.spacingLarge
                columns: 2
                rowSpacing: Tokens.spacingMedium
                columnSpacing: Tokens.spacingLarge

                Label { text: qsTr("Barcode"); color: Tokens.textPrimary }
                ColumnLayout {
                    Layout.fillWidth: true
                    TextField {
                        id: barcodeField
                        Layout.fillWidth: true
                        placeholderText: qsTr("Scan barcode or type manually")
                        enabled: !root.submitting
                        activeFocusOnTab: true
                        onTextEdited: root.markDirty()
                        onActiveFocusChanged: root.updateScanner()
                        onAccepted: sampleIdField.forceActiveFocus()
                    }
                    Label {
                        visible: root.barcodeError.length > 0
                        text: root.barcodeError
                        color: Tokens.danger
                    }
                }

                Label { text: qsTr("Sample ID"); color: Tokens.textPrimary }
                ColumnLayout {
                    Layout.fillWidth: true
                    TextField {
                        id: sampleIdField
                        Layout.fillWidth: true
                        placeholderText: qsTr("Unique sample identifier")
                        enabled: !root.submitting
                        onTextEdited: root.markDirty()
                    }
                    Label {
                        visible: root.sampleIdError.length > 0
                        text: root.sampleIdError
                        color: Tokens.danger
                    }
                }

                Label { text: qsTr("Sample type"); color: Tokens.textPrimary }
                ComboBox {
                    id: sampleTypeBox
                    Layout.fillWidth: true
                    model: [qsTr("Serum"), qsTr("Plasma"), qsTr("Whole Blood")]
                    enabled: !root.submitting
                    onActivated: root.markDirty()
                }

                Label { text: qsTr("Rack"); color: Tokens.textPrimary }
                ColumnLayout {
                    Layout.fillWidth: true
                    TextField {
                        id: rackField
                        Layout.fillWidth: true
                        text: "R1"
                        enabled: !root.submitting
                        onTextEdited: root.markDirty()
                    }
                    Label {
                        visible: root.rackError.length > 0
                        text: root.rackError
                        color: Tokens.danger
                    }
                }

                Label { text: qsTr("Position"); color: Tokens.textPrimary }
                SpinBox {
                    id: positionBox
                    from: 1
                    to: 100
                    value: 1
                    editable: true
                    enabled: !root.submitting
                    onValueModified: root.markDirty()
                }

                Label { text: qsTr("Priority"); color: Tokens.textPrimary }
                ComboBox {
                    id: priorityBox
                    Layout.fillWidth: true
                    model: [qsTr("Routine"), qsTr("STAT")]
                    enabled: !root.submitting
                    onActivated: root.markDirty()
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true
            Label {
                Layout.fillWidth: true
                text: root.statusMessage
                color: root.statusMessage.indexOf(qsTr("registered")) >= 0
                       ? Tokens.success : Tokens.textSecondary
                elide: Text.ElideRight
            }
            Button {
                text: qsTr("Reset")
                enabled: !root.submitting
                onClicked: root.resetForm()
            }
            Button {
                text: root.submitting ? qsTr("Submitting...") : qsTr("Submit")
                enabled: !root.submitting
                onClicked: root.submit()
            }
        }
    }

    Timer {
        id: submitTimer
        interval: 650
        repeat: false
        onTriggered: {
            var barcode = barcodeField.text.trim()
            var sampleId = sampleIdField.text.trim()
            sampleModel.appendRow({
                sampleId: sampleId,
                barcode: barcode,
                sampleType: sampleTypeBox.currentText,
                rack: rackField.text.trim(),
                position: positionBox.value,
                priority: priorityBox.currentText,
                status: "Pending",
                testCount: 1,
                result: "",
                createTime: "Now"
            })
            if (root.busyToken.length > 0)
                overlayCoordinator.endBusy(root.busyToken)
            root.busyToken = ""
            root.submitting = false
            root.statusMessage = qsTr("Sample %1 registered successfully.").arg(sampleId)
            root.resetForm()
            root.statusMessage = qsTr("Sample %1 registered successfully.").arg(sampleId)
        }
    }

    Connections {
        target: scanner.session
        function onScanAccepted(value) {
            if (!root.pageActive)
                return
            barcodeField.text = value
            if (sampleIdField.text.trim().length === 0)
                sampleIdField.text = "S-" + value
            root.statusMessage = qsTr("Scanner accepted %1").arg(value)
            root.markDirty()
            sampleIdField.forceActiveFocus()
            root.updateScanner()
        }
        function onScanRejected(rawValue, reason) {
            if (!root.pageActive)
                return
            root.statusMessage = qsTr("Scanner rejected %1: %2").arg(rawValue).arg(reason)
        }
    }

    Shortcut {
        sequence: "F2"
        onActivated: focusNavigator.moveNext()
    }
}
