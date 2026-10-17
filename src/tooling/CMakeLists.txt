qt_add_library(LfQuickTooling)
add_library(LfQuick::Tooling ALIAS LfQuickTooling)

target_compile_definitions(LfQuickTooling PRIVATE LFQUICKTOOLING_LIBRARY)

target_include_directories(LfQuickTooling
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

qt_add_qml_module(LfQuickTooling
    URI LfQuick.Tooling
    VERSION 1.0
    RESOURCE_PREFIX /qt/qml
    SOURCES
        include/LfQuick/Tooling/QmlHotReloadController.h
        include/LfQuick/Tooling/QmlHotReloadHost.h
        src/QmlHotReloadController.cpp
        src/QmlHotReloadHost.cpp
)

target_sources(LfQuickTooling PUBLIC
    FILE_SET HEADERS
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include
    FILES
        include/LfQuick/Tooling/LfQuickToolingGlobal.h
        include/LfQuick/Tooling/QmlHotReloadController.h
        include/LfQuick/Tooling/QmlHotReloadHost.h
)

target_link_libraries(LfQuickTooling
    PUBLIC
        LfQuick::Ui
        Qt6::Core
        Qt6::Qml
        Qt6::QmlIntegration
        Qt6::Quick
)
