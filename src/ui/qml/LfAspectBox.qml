import QtQuick

Item {
    id: root

    enum FitMode {
        Contain,
        Cover,
        FitWidth,
        FitHeight
    }

    property real aspectRatio: 1.0
    property int fitMode: LfAspectBox.Contain

    default property alias contentData: content.data
    readonly property alias contentItem: content

    implicitWidth: content.childrenRect.width
    implicitHeight: aspectRatio > 0 && implicitWidth > 0
                    ? implicitWidth / aspectRatio
                    : content.childrenRect.height

    function targetSize() {
        var ratio = Math.max(0.000001, root.aspectRatio)
        var availableWidth = Math.max(0, root.width)
        var availableHeight = Math.max(0, root.height)

        if (root.fitMode === LfAspectBox.FitWidth)
            return Qt.size(availableWidth, availableWidth / ratio)
        if (root.fitMode === LfAspectBox.FitHeight)
            return Qt.size(availableHeight * ratio, availableHeight)

        var availableRatio = availableHeight > 0 ? availableWidth / availableHeight : ratio
        var fitWidth = root.fitMode === LfAspectBox.Contain
                ? availableRatio <= ratio
                : availableRatio >= ratio
        return fitWidth
                ? Qt.size(availableWidth, availableWidth / ratio)
                : Qt.size(availableHeight * ratio, availableHeight)
    }

    Item {
        id: content

        readonly property size calculatedSize: root.targetSize()

        width: calculatedSize.width
        height: calculatedSize.height
        x: (root.width - width) / 2
        y: (root.height - height) / 2
    }
}
