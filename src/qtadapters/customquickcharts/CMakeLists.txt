qt_add_library(LfQuickCustomQuickCharts)
add_library(LfQuick::CustomQuickCharts ALIAS LfQuickCustomQuickCharts)

qt_add_qml_module(LfQuickCustomQuickCharts
    URI LfQuick.Charts.CustomQuick
    VERSION 1.0
    RESOURCE_PREFIX /qt/qml
    QML_FILES
        qml/CustomQuickBackend.qml
)

target_link_libraries(LfQuickCustomQuickCharts
    PUBLIC
        LfQuick::Charts
        Qt6::Qml
        Qt6::QmlIntegration
        Qt6::Quick
)
