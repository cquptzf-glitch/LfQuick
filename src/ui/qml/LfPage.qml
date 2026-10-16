
import QtQuick

Item {
    id: root

    signal activated()
    signal deactivated()

    function onPageActivated() { activated() }
    function onPageDeactivated() { deactivated() }
}
