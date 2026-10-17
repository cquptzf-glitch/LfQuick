qt_add_library(LfQuickCanvasPainterAdapter)
add_library(LfQuick::CanvasPainterAdapter ALIAS LfQuickCanvasPainterAdapter)

qt_add_qml_module(LfQuickCanvasPainterAdapter
    URI LfQuick.CanvasPainter
    VERSION 1.0
    RESOURCE_PREFIX /qt/qml
    OUTPUT_TARGETS _lfquick_canvaspainter_qml_output_targets
    QML_FILES
        qml/Canvas2DSurface.qml
        qml/CanvasPainterBackend.qml
)
lfquick_register_qml_output_targets(
    LfQuickCanvasPainterAdapter ${_lfquick_canvaspainter_qml_output_targets})

target_link_libraries(LfQuickCanvasPainterAdapter
    PUBLIC
        Qt6::CanvasPainter
        Qt6::Qml
        Qt6::QmlIntegration
        Qt6::Quick
)
