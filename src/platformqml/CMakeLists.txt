qt_add_library(LfQuickPlatformQml)
add_library(LfQuick::PlatformQml ALIAS LfQuickPlatformQml)

qt_add_qml_module(LfQuickPlatformQml
    URI LfQuick.Platform
    VERSION 1.0
    RESOURCE_PREFIX /qt/qml
    OUTPUT_TARGETS _lfquick_platform_qml_output_targets
    SOURCES
        PlatformQmlTypes.h
    QML_FILES
        ScannerTarget.qml
        InputSourceRegistration.qml
)
lfquick_register_qml_output_targets(
    LfQuickPlatformQml ${_lfquick_platform_qml_output_targets})

target_link_libraries(LfQuickPlatformQml
    PUBLIC
        LfQuick::Platform
        Qt6::Qml
        Qt6::QmlIntegration
)
