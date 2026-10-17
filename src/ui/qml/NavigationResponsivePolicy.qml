import QtQuick

QtObject {
    id: root

    enum Mode { Expanded, Compact, Hidden }

    property real width: 0
    property real compactBreakpoint: 1080
    property real hiddenBreakpoint: 720
    property bool allowHidden: true

    readonly property int mode: allowHidden && width > 0 && width < hiddenBreakpoint
                                ? NavigationResponsivePolicy.Hidden
                                : width > 0 && width < compactBreakpoint
                                  ? NavigationResponsivePolicy.Compact
                                  : NavigationResponsivePolicy.Expanded
    readonly property bool compact: mode === NavigationResponsivePolicy.Compact
    readonly property bool visible: mode !== NavigationResponsivePolicy.Hidden
}
