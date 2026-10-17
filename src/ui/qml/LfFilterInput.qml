import QtQuick

LfSearchBox {
    id: control

    signal filterAccepted(string filterText)

    placeholderText: qsTr("Filter")
    accessibilityName: placeholderText.length > 0 ? placeholderText : qsTr("Filter")

    onAccepted: filterAccepted(text.trimmed())
}
