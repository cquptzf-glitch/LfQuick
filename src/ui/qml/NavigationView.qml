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

                required property int index
                required property string route
                required property string label
                required property string icon
                required property string section
                required property string group
                required property var badge
                required property bool itemVisible
                required property bool itemEnabled
                required property bool selected
                required property var parameters

                width: ListView.view.width
                visible: itemVisible
                height: visible ? contentColumn.implicitHeight : 0

                readonly property bool showGroupHeader: group.length > 0
                    && (index === 0
                        || !root.model
                        || root.model.get(index - 1).group !== group
                        || root.model.get(index - 1).section !== section)
                readonly property bool showSectionDivider: index > 0
                    && root.model
                    && root.model.get(index - 1).section !== section

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
                        text: delegateRoot.group
                        color: Tokens.textSecondary
                        font.pixelSize: 12
                        elide: Text.ElideRight
                        leftPadding: 8
                        rightPadding: 8
                    }

                    Button {
                        id: navigationButton
                        Layout.fillWidth: true
                        checkable: true
                        checked: delegateRoot.selected
                        enabled: delegateRoot.itemEnabled
                        leftPadding: 10
                        rightPadding: 10

                        contentItem: RowLayout {
                            spacing: 8

                            Image {
                                visible: delegateRoot.icon.length > 0
                                source: delegateRoot.icon
                                sourceSize.width: 18
                                sourceSize.height: 18
                                Layout.preferredWidth: visible ? 18 : 0
                                Layout.preferredHeight: visible ? 18 : 0
                                fillMode: Image.PreserveAspectFit
                            }

                            Label {
                                Layout.fillWidth: true
                                visible: !root.compact
                                text: delegateRoot.label
                                color: navigationButton.enabled ? Tokens.textPrimary : Tokens.textSecondary
                                elide: Text.ElideRight
                            }

                            Label {
                                visible: root.compact && delegateRoot.icon.length === 0
                                text: delegateRoot.label.length > 0 ? delegateRoot.label.slice(0, 1) : "•"
                                color: navigationButton.enabled ? Tokens.textPrimary : Tokens.textSecondary
                                horizontalAlignment: Text.AlignHCenter
                                Layout.fillWidth: true
                            }

                            Label {
                                visible: delegateRoot.badge !== undefined
                                    && delegateRoot.badge !== null
                                    && String(delegateRoot.badge).length > 0
                                text: String(delegateRoot.badge)
                                color: Tokens.textSecondary
                                font.pixelSize: 11
                            }
                        }

                        onClicked: root.navigate(delegateRoot.route, delegateRoot.parameters)
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
