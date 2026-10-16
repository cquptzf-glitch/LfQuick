import QtQuick

Item {
    id: root

    property string title: ""

    signal activated()
    signal deactivated()
    signal suspended()
    signal resumed()

    function onPageActivated() { activated() }
    function onPageDeactivated() { deactivated() }
    function onPageSuspended() { suspended() }
    function onPageResumed() { resumed() }
}
