import QtQuick

Item {
    id: root

    property Item source
    property bool preferImplicitSize: true
    property real fallbackWidth: 0
    property real fallbackHeight: 0

    implicitWidth: source
                   ? (preferImplicitSize && source.implicitWidth > 0
                      ? source.implicitWidth : source.width)
                   : fallbackWidth
    implicitHeight: source
                    ? (preferImplicitSize && source.implicitHeight > 0
                       ? source.implicitHeight : source.height)
                    : fallbackHeight

    width: implicitWidth
    height: implicitHeight
    visible: false
}
