import QtQuick
import QtQuick.Controls as Controls

Controls.Control {
    id: root

    property int pageCount: 1
    property int currentPage: 1
    property int maxVisiblePages: 7
    property bool showFirstLast: true

    signal pageRequested(int page)

    readonly property int normalizedPageCount: Math.max(1, pageCount)
    readonly property int visibleCount: Math.min(normalizedPageCount, Math.max(1, maxVisiblePages))
    readonly property int firstVisiblePage: Math.max(
        1,
        Math.min(currentPage - Math.floor(visibleCount / 2), normalizedPageCount - visibleCount + 1))

    function requestPage(page) {
        const target = Math.max(1, Math.min(normalizedPageCount, page))
        if (target === currentPage)
            return
        currentPage = target
        pageRequested(target)
    }

    onPageCountChanged: {
        if (currentPage > normalizedPageCount)
            currentPage = normalizedPageCount
    }

    implicitWidth: row.implicitWidth + leftPadding + rightPadding
    implicitHeight: Math.max(Tokens.controlHeight, row.implicitHeight)
    leftPadding: Units.xs
    rightPadding: Units.xs
    topPadding: 0
    bottomPadding: 0

    contentItem: Row {
        id: row
        spacing: Units.xs

        LfToolButton {
            visible: root.showFirstLast
            compact: true
            text: "«"
            enabled: root.currentPage > 1
            Accessible.name: qsTr("First page")
            onClicked: root.requestPage(1)
        }

        LfToolButton {
            compact: true
            text: "‹"
            enabled: root.currentPage > 1
            Accessible.name: qsTr("Previous page")
            onClicked: root.requestPage(root.currentPage - 1)
        }

        Repeater {
            model: root.visibleCount

            LfToolButton {
                required property int index
                readonly property int pageNumber: root.firstVisiblePage + index
                compact: false
                text: String(pageNumber)
                checkable: true
                checked: pageNumber === root.currentPage
                Accessible.name: qsTr("Page %1").arg(pageNumber)
                onClicked: root.requestPage(pageNumber)
            }
        }

        LfToolButton {
            compact: true
            text: "›"
            enabled: root.currentPage < root.normalizedPageCount
            Accessible.name: qsTr("Next page")
            onClicked: root.requestPage(root.currentPage + 1)
        }

        LfToolButton {
            visible: root.showFirstLast
            compact: true
            text: "»"
            enabled: root.currentPage < root.normalizedPageCount
            Accessible.name: qsTr("Last page")
            onClicked: root.requestPage(root.normalizedPageCount)
        }
    }

    background: Item { }
}
