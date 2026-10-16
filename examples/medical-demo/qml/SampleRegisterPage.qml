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

    readonly property var barcodeState: formValidation.field("barcode", "sample.barcode")
    readonly property var sampleIdState: formValidation.field("sampleId", "sample.id")
    readonly property var rackState: formValidation.field("rack", "sample.rack")

    ValidationRuleTable {
        id: validationRules
        rules: ({
            "sample.barcode": {
                "required": true,
                "minLength": 4,
                "maxLength": 32,
                "regularExpression": "[A-Za-z0-9_-]+"
            },
            "sample.id": {
                "required": true,
                "minLength": 3,
                "maxLength": 24,
                "regularExpression": "[A-Za-z0-9_-]+"
            },
            "sample.rack": {
                "required": true,
                "minLength": 1,
                "maxLength": 12,
                "regularExpression": "[A-Za-z0-9_-]+"
            }
        })
    }

    ValidationFormState {
        id: formValidation
        ruleTable: validationRules
    }

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

    function validateBarcode() {
        var value = barcodeField.text.trim()
        formValidation.clearExternalError("barcode")
        var valid = formValidation.validateField("barcode", value)
        if (valid && sampleModel.findFirst("barcode", value, Qt.CaseInsensitive) >= 0) {
            formValidation.setExternalError("barcode", qsTr("Barcode already exists"))
            valid = false
        }
        return valid
    }

    function validateSampleId() {
        var value = sampleIdField.text.trim()
        formValidation.clearExternalError("sampleId")
        var valid = formValidation.validateField("sampleId", value)
        if (valid && sampleModel.findFirst("sampleId", value, Qt.CaseInsensitive) >= 0) {
            formValidation.setExternalError("sampleId", qsTr("Sample ID already exists"))
            valid = false
        }
        return valid
    }

    function validateRack() {
        formValidation.clearExternalError("rack")
        return formValidation.validateField("rack", rackField.text.trim())
    }

    function validateForm() {
        root.validateBarcode()
        root.validateSampleId()
        root.validateRack()
        if (!formValidation.valid)
            root.statusMessage = qsTr("Please correct %1 invalid field(s) before submitting.")
                    .arg(formValidation.errorCount)
        return formValidation.valid
    }

    function resetForm() {
        root.suppressDirty = true
        barcodeField.text = ""
        sampleIdField.text = ""
        sampleTypeBox.currentIndex = 0
        rackField.text = "R1"
        positionBox.value = 1
        priorityBox.currentIndex = 0
        formValidation.reset()
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
                        onTextEdited: {
                            root.markDirty()
                            if (root.barcodeState.touched)
                                root.validateBarcode()
                        }
                        onActiveFocusChanged: root.updateScanner()
                        onAccepted: sampleIdField.forceActiveFocus()
                    }
                    Label {
                        visible: root.barcodeState.hasError
                        text: root.barcodeState.message
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
                        onTextEdited: {
                            root.markDirty()
                            if (root.sampleIdState.touched)
                                root.validateSampleId()
                        }
                    }
                    Label {
                        visible: root.sampleIdState.hasError
                        text: root.sampleIdState.message
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
                        onTextEdited: {
                            root.markDirty()
                            if (root.rackState.touched)
                                root.validateRack()
                        }
                    }
                    Label {
                        visible: root.rackState.hasError
                        text: root.rackState.message
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
            if (root.barcodeState.touched)
                root.validateBarcode()
            if (root.sampleIdState.touched)
                root.validateSampleId()
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
