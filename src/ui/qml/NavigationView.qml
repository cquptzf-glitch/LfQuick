import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: root

    property var model
    property var runtime
    property bool compact: false
    property int compactWidth: 72
    property int expandedWidth: 220
    property bool showBackButton: true

    readonly property int navigationWidth: compact ? compactWidth : expandedWidth

    signal routeRequested(string route, var parameters)
    signal backRequested()

    implicitWidth: navigationWidth
    implicitHeight: 480

    function syncSelectedRoute() {
        if (root.model && root.runtime && root.model.selectedRoute !== undefined)
            root.model.selectedRoute = root.runtime.currentRoute
    }

    function navigate(route, parameters) {
        if (root.runtime)
            root.runtime.push(route, parameters || {})
        else
            root.routeRequested(route, parameters || {})
    }

    Rectangle {
        anchors.fill: parent
        color: Tokens.surface
        border.color: Tokens.border
        border.width: 1
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 8
        spacing: 6

        ToolButton {
            Layout.fillWidth: true
            visible: root.showBackButton
            enabled: root.runtime ? root.runtime.canGoBack : false
            text: root.compact ? "‹" : qsTr("Back")
            onClicked: {
                if (root.runtime)
                    root.runtime.back()
                else
                    root.backRequested()
            }
        }

        ListView {
            id: listView
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            spacing: 4
            model: root.model

            delegate: Item {
                id: delegateRoot
                width: ListView.view.width
                visible: model.itemVisible
                height: visible ? contentColumn.implicitHeight : 0

                readonly property bool showGroupHeader: model.group.length > 0
                    && (index === 0
                        || !root.model
                        || root.model.get(index - 1).group !== model.group
                        || root.model.get(index - 1).section !== model.section)
                readonly property bool showSectionDivider: index > 0
                    && root.model
                    && root.model.get(index - 1).section !== model.section

                ColumnLayout {
                    id: contentColumn
                    width: parent.width
                    spacing: 4

                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: delegateRoot.showSectionDivider ? 1 : 0
                        visible: delegateRoot.showSectionDivider
                        color: Tokens.border
                    }

                    Label {
                        Layout.fillWidth: true
                        visible: delegateRoot.showGroupHeader && !root.compact
                        text: model.group
                        color: Tokens.textSecondary
                        font.pixelSize: 12
                        elide: Text.ElideRight
                        leftPadding: 8
                        rightPadding: 8
                    }

                    Button {
                        Layout.fillWidth: true
                        checkable: true
                        checked: model.selected
                        enabled: model.itemEnabled
                        leftPadding: 10
                        rightPadding: 10

                        contentItem: RowLayout {
                            spacing: 8

                            Image {
                                visible: model.icon.length > 0
                                source: model.icon
                                sourceSize.width: 18
                                sourceSize.height: 18
                                Layout.preferredWidth: visible ? 18 : 0
                                Layout.preferredHeight: visible ? 18 : 0
                                fillMode: Image.PreserveAspectFit
                            }

                            Label {
                                Layout.fillWidth: true
                                visible: !root.compact
                                text: model.label
                                color: parent.parent.enabled ? Tokens.textPrimary : Tokens.textSecondary
                                elide: Text.ElideRight
                            }

                            Label {
                                visible: root.compact && model.icon.length === 0
                                text: model.label.length > 0 ? model.label.slice(0, 1) : "•"
                                color: Tokens.textPrimary
                                horizontalAlignment: Text.AlignHCenter
                                Layout.fillWidth: true
                            }

                            Label {
                                visible: model.badge !== undefined
                                    && model.badge !== null
                                    && String(model.badge).length > 0
                                text: String(model.badge)
                                color: Tokens.textSecondary
                                font.pixelSize: 11
                            }
                        }

                        onClicked: root.navigate(model.route, model.parameters)
                    }
                }
            }
        }
    }

    Connections {
        target: root.runtime
        function onCurrentRouteChanged() { root.syncSelectedRoute() }
    }

    Component.onCompleted: syncSelectedRoute()
}
