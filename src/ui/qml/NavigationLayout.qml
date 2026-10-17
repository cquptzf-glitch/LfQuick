import QtQuick
import QtQuick.Layouts

Item {
    id: root

    property Item navigationItem
    property Item contentItem
    property bool navigationVisible: true
    property int navigationWidth: 220
    property int spacing: 0

    implicitWidth: 1000
    implicitHeight: 640

    RowLayout {
        anchors.fill: parent
        spacing: root.spacing

        Item {
            id: navigationSlot
            visible: root.navigationVisible
            Layout.preferredWidth: visible ? root.navigationWidth : 0
            Layout.fillHeight: true
        }

        Item {
            id: contentSlot
            Layout.fillWidth: true
            Layout.fillHeight: true
        }
    }

    onNavigationItemChanged: {
        if (navigationItem)
            navigationItem.parent = navigationSlot
    }
    onContentItemChanged: {
        if (contentItem)
            contentItem.parent = contentSlot
    }

    Component.onCompleted: {
        if (navigationItem)
            navigationItem.parent = navigationSlot
        if (contentItem)
            contentItem.parent = contentSlot
    }
}
