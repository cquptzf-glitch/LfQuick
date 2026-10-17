qt_add_library(LfQuickCustomQuickCharts)
add_library(LfQuick::CustomQuickCharts ALIAS LfQuickCustomQuickCharts)

qt_add_qml_module(LfQuickCustomQuickCharts
    URI LfQuick.Charts.CustomQuick
    VERSION 1.0
    RESOURCE_PREFIX /qt/qml
    OUTPUT_TARGETS _lfquick_custom_quick_charts_qml_output_targets
    QML_FILES
        qml/CustomQuickBackend.qml
)
lfquick_register_qml_output_targets(
    LfQuickCustomQuickCharts ${_lfquick_custom_quick_charts_qml_output_targets})

target_link_libraries(LfQuickCustomQuickCharts
    PUBLIC
        LfQuick::Charts
        Qt6::Qml
        Qt6::QmlIntegration
        Qt6::Quick
)
