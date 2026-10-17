qt_add_library(LfQuickIntegration)
add_library(LfQuick::Integration ALIAS LfQuickIntegration)

target_compile_definitions(LfQuickIntegration PRIVATE LFQUICKINTEGRATION_LIBRARY)

target_include_directories(LfQuickIntegration
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

qt_add_qml_module(LfQuickIntegration
    URI LfQuick.Integration
    VERSION 1.0
    RESOURCE_PREFIX /qt/qml
    SOURCES
        include/LfQuick/Integration/LfScannerTarget.h
        src/LfScannerTarget.cpp
)

target_sources(LfQuickIntegration PUBLIC
    FILE_SET HEADERS
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include
    FILES
        include/LfQuick/Integration/LfQuickIntegrationGlobal.h
        include/LfQuick/Integration/LfScannerTarget.h
)

target_link_libraries(LfQuickIntegration
    PUBLIC
        LfQuick::Ui
        LfQuick::Platform
        Qt6::Core
        Qt6::Qml
        Qt6::QmlIntegration
        Qt6::Quick
)
