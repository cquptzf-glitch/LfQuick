qt_add_library(LfQuickSafeAreaAdapter)
add_library(LfQuick::SafeAreaAdapter ALIAS LfQuickSafeAreaAdapter)

qt_add_qml_module(LfQuickSafeAreaAdapter
    URI LfQuick.SafeArea
    VERSION 1.0
    RESOURCE_PREFIX /qt/qml
    OUTPUT_TARGETS _lfquick_safearea_qml_output_targets
    QML_FILES
        qml/SafeAreaHost.qml
        qml/SafeAreaWindow.qml
)
lfquick_register_qml_output_targets(
    LfQuickSafeAreaAdapter ${_lfquick_safearea_qml_output_targets})

target_link_libraries(LfQuickSafeAreaAdapter
    PUBLIC
        Qt6::Qml
        Qt6::QmlIntegration
        Qt6::Quick
        Qt6::QuickControls2
)
