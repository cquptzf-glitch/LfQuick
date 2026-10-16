
import QtQuick

Item {
    id: root

    property alias pageItem: pageLoader.item
    signal loadFailed(string message)

    function loadPage(url, properties) {
        pageLoader.setSource(url, properties ?? {})
    }

    function clearPage() {
        pageLoader.source = ""
    }

    Loader {
        id: pageLoader
        anchors.fill: parent

        onStatusChanged: {
            if (status === Loader.Error)
                root.loadFailed("Failed to load page: " + source)
        }
    }
}
