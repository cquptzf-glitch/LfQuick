qt_add_library(LfQuickDiagnostics)
add_library(LfQuick::Diagnostics ALIAS LfQuickDiagnostics)

target_compile_definitions(LfQuickDiagnostics PRIVATE LFQUICKDIAGNOSTICS_LIBRARY)

target_include_directories(LfQuickDiagnostics
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

qt_add_qml_module(LfQuickDiagnostics
    URI LfQuick.Diagnostics
    VERSION 1.0
    RESOURCE_PREFIX /qt/qml
    SOURCES
        include/LfQuick/Diagnostics/DiagnosticsModel.h
        src/DiagnosticsModel.cpp
    QML_FILES
        qml/DiagnosticsHud.qml
)

target_sources(LfQuickDiagnostics PUBLIC
    FILE_SET HEADERS
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include
    FILES
        include/LfQuick/Diagnostics/LfQuickDiagnosticsGlobal.h
        include/LfQuick/Diagnostics/DiagnosticsModel.h
)

target_link_libraries(LfQuickDiagnostics
    PUBLIC
        Qt6::Core
        Qt6::Qml
        Qt6::QmlIntegration
        Qt6::Quick
)
