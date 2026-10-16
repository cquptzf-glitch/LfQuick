qt_add_executable(LfQuickHybridDemo
    cpp/main.cpp
)

target_link_libraries(LfQuickHybridDemo
    PRIVATE
        LfQuick::Core
        LfQuick::Runtime
        LfQuick::Ui
        LfQuick::WidgetBridge
        Qt6::Widgets
        Qt6::Qml
        Qt6::Quick
        Qt6::QuickControls2
)

qt_add_qml_module(LfQuickHybridDemo
    URI LfQuick.HybridDemo
    VERSION 1.0
    RESOURCE_PREFIX /qt/qml
    QML_FILES
        qml/HybridQuickPage.qml
)
