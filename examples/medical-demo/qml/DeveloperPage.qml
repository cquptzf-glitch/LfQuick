import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import LfQuick

LfPage {
    id: root
    title: qsTr("Developer")

    property int demoProgress: 42
    property int demoPage: 3
    property date demoDateTime: new Date(2026, 9, 17, 10, 30)

    FocusNavigator {
        id: focusNavigator
        scope: root
    }

    ScrollView {
        id: developerScroll
        anchors.fill: parent
        clip: true
        ScrollBar.horizontal.policy: ScrollBar.AlwaysOff

        ColumnLayout {
            width: Math.max(0, developerScroll.availableWidth)
            spacing: Tokens.spacingMedium

            Item { Layout.preferredHeight: Tokens.spacingSmall }

            Label {
                Layout.leftMargin: Tokens.spacingLarge
                Layout.rightMargin: Tokens.spacingLarge
                text: qsTr("LfQuick Developer Playground")
                font.pixelSize: 28
                font.bold: true
                color: Tokens.textPrimary
            }

            Label {
                Layout.fillWidth: true
                Layout.leftMargin: Tokens.spacingLarge
                Layout.rightMargin: Tokens.spacingLarge
                text: qsTr("Framework verification: design system, overlay, focus, scanner, DataGrid and diagnostics.")
                wrapMode: Text.WordWrap
                color: Tokens.textSecondary
            }

            RowLayout {
                Layout.fillWidth: true
                Layout.leftMargin: Tokens.spacingLarge
                Layout.rightMargin: Tokens.spacingLarge
                spacing: Tokens.spacingSmall

                LfButton {
                    text: Theme.dark ? qsTr("Switch to Light") : qsTr("Switch to Dark")
                    highlighted: true
                    onClicked: Theme.dark ? Theme.useLight() : Theme.useDark()
                }

                LfButton {
                    text: qsTr("System Theme")
                    enabled: Theme.mode !== Theme.System
                    onClicked: Theme.useSystem()
                }

                LfSwitch {
                    text: qsTr("High Contrast")
                    checked: Theme.highContrast
                    onToggled: Theme.highContrast = checked
                }

                LfButton {
                    text: qsTr("Start Busy")
                    onClicked: {
                        const token = overlayCoordinator.beginBusy(qsTr("Developer operation..."), true)
                        developerBusyTimer.token = token
                        developerBusyTimer.restart()
                    }
                }

                LfButton {
                    text: qsTr("Simulate Scanner")
                    onClicked: scanner.simulateScan("DEV-SCAN-20261016")
                }

                LfButton {
                    text: qsTr("Next Focus")
                    onClicked: focusNavigator.moveNext()
                }

                Item { Layout.fillWidth: true }

                LfComboBox {
                    id: densityPicker
                    Layout.preferredWidth: 130
                    model: [qsTr("Compact"), qsTr("Standard"), qsTr("Touch")]
                    currentIndex: Theme.density === Theme.Compact ? 0
                                : Theme.density === Theme.Touch ? 2 : 1
                    onActivated: function(index) {
                        Theme.density = index === 0 ? Theme.Compact
                                      : index === 2 ? Theme.Touch
                                                    : Theme.Standard
                    }
                }
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.leftMargin: Tokens.spacingLarge
                Layout.rightMargin: Tokens.spacingLarge
                Layout.preferredHeight: controlsLayout.implicitHeight + Tokens.spacingLarge * 2
                radius: Tokens.radiusMedium
                color: Tokens.surfaceSecondary
                border.color: Tokens.border

                ColumnLayout {
                    id: controlsLayout
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.top: parent.top
                    anchors.margins: Tokens.spacingLarge
                    spacing: Tokens.spacingMedium

                    Label {
                        text: qsTr("Style-agnostic base controls")
                        font.bold: true
                        color: Tokens.textPrimary
                    }

                    GridLayout {
                        Layout.fillWidth: true
                        columns: 4
                        columnSpacing: Tokens.spacingMedium
                        rowSpacing: Tokens.spacingMedium

                        LfTextField {
                            Layout.fillWidth: true
                            placeholderText: qsTr("Text field")
                            text: "LfQuick"
                        }

                        LfNumberInput {
                            Layout.fillWidth: true
                            from: 0
                            to: 100
                            value: root.demoProgress
                            onValueModified: root.demoProgress = value
                        }

                        LfSearchBox {
                            Layout.fillWidth: true
                            placeholderText: qsTr("Search samples")
                        }

                        LfFilterInput {
                            Layout.fillWidth: true
                            placeholderText: qsTr("Filter status")
                        }

                        LfPasswordField {
                            Layout.fillWidth: true
                            placeholderText: qsTr("Password")
                            text: "demo-password"
                        }

                        LfComboBox {
                            Layout.fillWidth: true
                            model: [qsTr("Ready"), qsTr("Running"), qsTr("Review")]
                        }

                        LfCheckBox {
                            text: qsTr("Checked option")
                            checked: true
                        }

                        LfRadioButton {
                            text: qsTr("Radio option")
                            checked: true
                        }

                        LfSwitch {
                            text: qsTr("Animation")
                            checked: Theme.animationEnabled
                            onToggled: Theme.animationEnabled = checked
                        }

                        LfStatusIndicator {
                            status: LfStatusIndicator.Success
                            text: qsTr("Instrument ready")
                        }

                        LfProgressRing {
                            running: true
                        }

                        Item { Layout.fillWidth: true }

                        LfSlider {
                            Layout.columnSpan: 2
                            Layout.fillWidth: true
                            from: 0
                            to: 100
                            value: root.demoProgress
                            onMoved: root.demoProgress = Math.round(value)
                        }

                        LfRangeInput {
                            Layout.columnSpan: 2
                            Layout.fillWidth: true
                            from: 0
                            to: 100
                            first.value: 20
                            second.value: 80
                        }

                        LfProgressBar {
                            Layout.columnSpan: 3
                            Layout.fillWidth: true
                            from: 0
                            to: 100
                            value: root.demoProgress
                        }

                        LfButton {
                            text: qsTr("Advance %1%").arg(root.demoProgress)
                            onClicked: root.demoProgress = (root.demoProgress + 13) % 101
                        }

                        LfDatePicker {
                            Layout.fillWidth: true
                            value: root.demoDateTime
                            onAccepted: function(dateValue) {
                                root.demoDateTime = new Date(dateValue.getFullYear(), dateValue.getMonth(), dateValue.getDate(),
                                                             root.demoDateTime.getHours(), root.demoDateTime.getMinutes())
                            }
                        }

                        LfTimePicker {
                            Layout.fillWidth: true
                            value: root.demoDateTime
                            minuteStep: 5
                            onAccepted: function(timeValue) {
                                root.demoDateTime = new Date(root.demoDateTime.getFullYear(), root.demoDateTime.getMonth(),
                                                             root.demoDateTime.getDate(), timeValue.getHours(), timeValue.getMinutes())
                            }
                        }

                        LfDateTimePicker {
                            Layout.columnSpan: 2
                            Layout.fillWidth: true
                            value: root.demoDateTime
                            minuteStep: 5
                            onAccepted: function(dateValue) { root.demoDateTime = dateValue }
                        }

                        LfDateRangePicker {
                            Layout.columnSpan: 2
                            Layout.fillWidth: true
                            startDate: new Date(2026, 9, 1)
                            endDate: new Date(2026, 9, 17)
                        }

                        LfButton {
                            text: qsTr("Destructive")
                            destructive: true
                        }

                        LfButton {
                            text: qsTr("Disabled")
                            enabled: false
                        }
                    }
                }
            }

            LfCard {
                Layout.fillWidth: true
                Layout.leftMargin: Tokens.spacingLarge
                Layout.rightMargin: Tokens.spacingLarge
                title: qsTr("Reusable composition controls")
                subtitle: qsTr("Breadcrumb, paging, disclosure, empty/loading states, identity, tags and command surfaces.")

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: Tokens.spacingMedium

                    LfBreadcrumb {
                        Layout.fillWidth: true
                        model: [qsTr("Home"), qsTr("Samples"), qsTr("Result review")]
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: Tokens.spacingSmall

                        LfAvatar {
                            name: "Lf Quick"
                        }

                        LfBadge {
                            tone: LfBadge.Success
                            text: qsTr("Online")
                        }

                        LfTag {
                            text: qsTr("STAT")
                            tone: LfBadge.Warning
                            selected: true
                        }

                        LfTag {
                            text: qsTr("QC passed")
                            tone: LfBadge.Success
                            closable: true
                            onCloseRequested: visible = false
                        }

                        Item { Layout.fillWidth: true }
                    }

                    LfCommandBar {
                        Layout.fillWidth: true

                        LfToolButton {
                            text: qsTr("Refresh")
                        }

                        LfToolButton {
                            text: qsTr("Export")
                        }

                        Item { Layout.fillWidth: true }

                        LfButton {
                            text: qsTr("Run")
                            highlighted: true
                        }
                    }

                    LfPagination {
                        Layout.alignment: Qt.AlignHCenter
                        pageCount: 18
                        currentPage: root.demoPage
                        maxVisiblePages: 5
                        onPageRequested: function(page) { root.demoPage = page }
                    }

                    LfExpander {
                        Layout.fillWidth: true
                        title: qsTr("Advanced details")
                        description: qsTr("Expandable content keeps secondary information out of the primary workflow.")
                        expanded: true

                        LfInfoBar {
                            Layout.fillWidth: true
                            severity: LfInfoBar.Info
                            title: qsTr("Runtime note")
                            message: qsTr("This section exercises nested reusable components and animated layout changes.")
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: Tokens.spacingLarge

                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: Tokens.spacingSmall

                            Label {
                                text: qsTr("Skeleton loading")
                                color: Tokens.textPrimary
                                font.bold: true
                            }

                            LfSkeleton {
                                Layout.fillWidth: true
                                Layout.preferredHeight: 18
                            }

                            LfSkeleton {
                                Layout.fillWidth: true
                                Layout.preferredWidth: 260
                                Layout.preferredHeight: 18
                            }

                            LfSkeleton {
                                Layout.preferredWidth: 160
                                Layout.preferredHeight: 18
                            }
                        }

                        LfDivider {
                            orientation: LfDivider.Vertical
                            Layout.fillHeight: true
                        }

                        LfEmptyState {
                            Layout.fillWidth: true
                            title: qsTr("No pending review")
                            message: qsTr("All synthetic demo results have been reviewed.")
                            actionText: qsTr("Refresh")
                        }
                    }
                }
            }

            LfCard {
                Layout.fillWidth: true
                Layout.leftMargin: Tokens.spacingLarge
                Layout.rightMargin: Tokens.spacingLarge
                title: qsTr("Complex selection controls")
                subtitle: qsTr("Searchable options and stable-key multi-selection.")

                RowLayout {
                    Layout.fillWidth: true
                    spacing: Tokens.spacingMedium

                    LfSearchableComboBox {
                        Layout.preferredWidth: 240
                        model: [qsTr("Whole blood"), qsTr("Serum"),
                                qsTr("Plasma"), qsTr("Quality control")]
                        currentIndex: 0
                    }

                    LfMultiSelect {
                        Layout.preferredWidth: 300
                        model: [
                            { text: qsTr("STAT"), value: "stat" },
                            { text: qsTr("QC"), value: "qc" },
                            { text: qsTr("Calibration"), value: "cal" }
                        ]
                        selectedValues: ["qc"]
                    }

                    Item { Layout.fillWidth: true }
                }
            }

            FocusDebugPanel {
                Layout.fillWidth: true
                Layout.leftMargin: Tokens.spacingLarge
                Layout.rightMargin: Tokens.spacingLarge
                navigator: focusNavigator
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.leftMargin: Tokens.spacingLarge
                Layout.rightMargin: Tokens.spacingLarge
                Layout.preferredHeight: 150
                radius: Tokens.radiusMedium
                color: Tokens.surfaceSecondary
                border.color: Tokens.border

                Column {
                    anchors.centerIn: parent
                    spacing: Tokens.spacingSmall

                    Label {
                        text: qsTr("Runtime metrics")
                        font.bold: true
                        color: Tokens.textPrimary
                    }

                    Label {
                        text: qsTr("Samples: %1 | Results: %2 | Scanner: %3 | InputRouter: %4")
                            .arg(sampleModel.count)
                            .arg(resultModel.count)
                            .arg(scanner.enabled ? qsTr("armed") : qsTr("off"))
                            .arg(inputRouter.installed ? qsTr("installed") : qsTr("off"))
                        color: Tokens.textSecondary
                    }

                    Label {
                        text: qsTr("Focus items: %1 | Density: %2 | Theme: %3")
                            .arg(focusNavigator.focusChain().length)
                            .arg(densityPicker.currentText)
                            .arg(Theme.mode === Theme.System ? qsTr("System")
                                 : Theme.dark ? qsTr("Dark") : qsTr("Light"))
                        color: Tokens.textSecondary
                    }
                }
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.leftMargin: Tokens.spacingLarge
                Layout.rightMargin: Tokens.spacingLarge
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

                            Rectangle {
                                required property color modelData
                                width: 48
                                height: 24
                                radius: Tokens.radiusSmall
                                color: modelData
                                border.width: Theme.highContrast ? 1 : 0
                                border.color: Tokens.borderStrong
                            }
                        }
                    }
                }
            }

            Item { Layout.preferredHeight: Tokens.spacingLarge }
        }
    }

    Timer {
        id: developerBusyTimer
        property string token: ""
        interval: 1600
        repeat: false
        onTriggered: {
            if (token.length > 0) {
                overlayCoordinator.endBusy(token)
                token = ""
            }
        }
    }
}
