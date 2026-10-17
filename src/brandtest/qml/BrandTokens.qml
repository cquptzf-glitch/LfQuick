pragma Singleton
import QtQuick

QtObject {
    property bool dark: false
    property bool highContrast: false
    property color accent: "#7357D9"

    readonly property color surface: highContrast ? (dark ? "#000000" : "#FFFFFF")
        : dark ? "#17151D" : "#FBFAFF"
    readonly property color surfaceRaised: highContrast ? surface
        : dark ? "#24212D" : "#F1EDFF"
    readonly property color text: highContrast ? (dark ? "#FFFFFF" : "#000000")
        : dark ? "#F7F4FF" : "#241F31"
    readonly property color textSecondary: highContrast ? text
        : dark ? "#C9C1DD" : "#6F6680"
    readonly property color border: highContrast ? text
        : dark ? "#635A73" : "#CFC6DF"
    readonly property color control: highContrast ? surface
        : dark ? "#2A2634" : "#FFFFFF"
    readonly property color hover: highContrast ? control
        : dark ? "#383142" : "#F5F1FF"
    readonly property color pressed: highContrast ? control
        : dark ? "#201D29" : "#E9E1FF"
    readonly property color onAccent: "#FFFFFF"
    readonly property color destructive: "#C13B54"

    readonly property int radius: 12
    readonly property int controlHeight: 38
    readonly property int spacing: 10
    readonly property int fontSize: 14
}
