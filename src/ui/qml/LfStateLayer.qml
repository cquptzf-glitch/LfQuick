import QtQuick

Rectangle {
    id: root

    property var control
    property bool selected: false
    property bool checked: false
    property color stateColor: Tokens.accent

    readonly property bool controlEnabled: !control || control.enabled === undefined || control.enabled
    readonly property bool hovered: control && control.hovered !== undefined && control.hovered
    readonly property bool pressed: control && ((control.down !== undefined && control.down)
                                                   || (control.pressed !== undefined && control.pressed))
    readonly property bool focused: control && control.activeFocus !== undefined && control.activeFocus
    readonly property real stateOpacity: !controlEnabled ? 0.0
        : pressed ? 0.16
        : hovered ? 0.10
        : selected || checked ? 0.08
        : 0.0

    color: stateColor
    opacity: stateOpacity
    radius: Tokens.radiusSmall
    visible: opacity > 0
    enabled: false

    Behavior on opacity {
        NumberAnimation { duration: Tokens.animationFast }
    }
}
