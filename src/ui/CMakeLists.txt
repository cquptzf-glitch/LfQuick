
qt_add_library(LfQuickUi STATIC)
add_library(LfQuick::Ui ALIAS LfQuickUi)

qt_add_qml_module(LfQuickUi
    URI LfQuick
    VERSION 1.0
    RESOURCE_PREFIX /qt/qml
    QML_FILES
        qml/LfPage.qml
        qml/QuickHostRoot.qml
)

target_link_libraries(LfQuickUi
    PUBLIC
        LfQuick::Core
        LfQuick::Runtime
        Qt6::Gui
        Qt6::Qml
        Qt6::Quick
        Qt6::QuickControls2
        Qt6::QuickLayouts
)
