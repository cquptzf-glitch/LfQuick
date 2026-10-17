qt_add_library(LfQuickTooling)
add_library(LfQuick::Tooling ALIAS LfQuickTooling)

target_compile_definitions(LfQuickTooling PRIVATE LFQUICKTOOLING_LIBRARY)

target_include_directories(LfQuickTooling
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include/LfQuick/Tooling
)

qt_add_qml_module(LfQuickTooling
    URI LfQuick.Tooling
    VERSION 1.0
    RESOURCE_PREFIX /qt/qml
    OUTPUT_TARGETS _lfquick_tooling_qml_output_targets
    SOURCES
        include/LfQuick/Tooling/QmlHotReloadController.h
        include/LfQuick/Tooling/QmlHotReloadHost.h
        include/LfQuick/Tooling/HotReloadStateTransfer.h
        src/QmlHotReloadController.cpp
        src/QmlHotReloadHost.cpp
        src/HotReloadStateTransfer.cpp
)
lfquick_register_qml_output_targets(
    LfQuickTooling ${_lfquick_tooling_qml_output_targets})

target_sources(LfQuickTooling PUBLIC
    FILE_SET HEADERS
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include
    FILES
        include/LfQuick/Tooling/LfQuickToolingGlobal.h
        include/LfQuick/Tooling/QmlHotReloadController.h
        include/LfQuick/Tooling/QmlHotReloadHost.h
        include/LfQuick/Tooling/HotReloadStateTransfer.h
)

target_link_libraries(LfQuickTooling
    PUBLIC
        LfQuick::Ui
        Qt6::Core
        Qt6::Qml
        Qt6::QmlIntegration
        Qt6::Quick
)
