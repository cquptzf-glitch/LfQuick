qt_add_library(LfQuickDiagnostics)
add_library(LfQuick::Diagnostics ALIAS LfQuickDiagnostics)

target_compile_definitions(LfQuickDiagnostics PRIVATE LFQUICKDIAGNOSTICS_LIBRARY)

target_include_directories(LfQuickDiagnostics
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

set_source_files_properties(qml/DiagnosticsTokens.qml PROPERTIES QT_QML_SINGLETON_TYPE TRUE)

qt_add_qml_module(LfQuickDiagnostics
    URI LfQuick.Diagnostics
    VERSION 1.0
    RESOURCE_PREFIX /qt/qml
    OUTPUT_TARGETS _lfquick_diagnostics_qml_output_targets
    SOURCES
        include/LfQuick/Diagnostics/DiagnosticsModel.h
        src/DiagnosticsModel.cpp
        include/LfQuick/Diagnostics/DiagnosticsLogSink.h
        src/DiagnosticsLogSink.cpp
        include/LfQuick/Diagnostics/ObjectInspectorModel.h
        src/ObjectInspectorModel.cpp
        include/LfQuick/Diagnostics/AccessibilityAudit.h
        src/AccessibilityAudit.cpp
    QML_FILES
        qml/DiagnosticsTokens.qml
        qml/DiagnosticsHud.qml
        qml/DiagnosticsObjectInspector.qml
        qml/DiagnosticsFpsHud.qml
        qml/DiagnosticsFocusInspector.qml
        qml/DiagnosticsOverlayInspector.qml
        qml/DiagnosticsRouteInspector.qml
)
lfquick_register_qml_output_targets(
    LfQuickDiagnostics ${_lfquick_diagnostics_qml_output_targets})

target_sources(LfQuickDiagnostics PUBLIC
    FILE_SET HEADERS
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include
    FILES
        include/LfQuick/Diagnostics/LfQuickDiagnosticsGlobal.h
        include/LfQuick/Diagnostics/DiagnosticsModel.h
        include/LfQuick/Diagnostics/DiagnosticsLogSink.h
        include/LfQuick/Diagnostics/ObjectInspectorModel.h
        include/LfQuick/Diagnostics/AccessibilityAudit.h
)

target_link_libraries(LfQuickDiagnostics
    PUBLIC
        LfQuick::Core
        Qt6::Core
        Qt6::Gui
        Qt6::Qml
        Qt6::QmlIntegration
        Qt6::Quick
)
