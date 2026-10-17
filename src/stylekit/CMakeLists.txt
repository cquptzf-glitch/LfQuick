qt_add_library(LfQuickStyleKit)
add_library(LfQuick::StyleKit ALIAS LfQuickStyleKit)

target_compile_definitions(LfQuickStyleKit PRIVATE LFQUICKSTYLEKIT_LIBRARY)

target_include_directories(LfQuickStyleKit
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

qt_add_qml_module(LfQuickStyleKit
    URI LfQuick.StyleKit
    VERSION 1.0
    RESOURCE_PREFIX /qt/qml
    SOURCES
        include/LfQuick/StyleKit/StyleKitBridge.h
        src/StyleKitBridge.cpp
)

target_sources(LfQuickStyleKit PUBLIC
    FILE_SET HEADERS
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include
    FILES
        include/LfQuick/StyleKit/LfQuickStyleKitGlobal.h
        include/LfQuick/StyleKit/StyleKitBridge.h
)

target_link_libraries(LfQuickStyleKit
    PUBLIC
        Qt6::Core
        Qt6::Qml
        Qt6::QmlIntegration
        Qt6::Widgets
        Qt6::LabsStyleKit
)
