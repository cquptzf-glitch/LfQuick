add_library(LfQuickWidgetBridge
    src/QuickSurface.cpp
    src/WidgetPageProvider.cpp
    src/WidgetWindowProvider.cpp
    src/WidgetFocusBridge.cpp
    src/WidgetOverlayBackend.cpp
)
add_library(LfQuick::WidgetBridge ALIAS LfQuickWidgetBridge)

target_compile_definitions(LfQuickWidgetBridge PRIVATE LFQUICKWIDGETBRIDGE_LIBRARY)

target_include_directories(LfQuickWidgetBridge
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_link_libraries(LfQuickWidgetBridge
    PUBLIC
        LfQuick::Runtime
        LfQuick::Ui
        Qt6::Widgets
        Qt6::QuickWidgets
)

target_sources(LfQuickWidgetBridge PUBLIC
    FILE_SET HEADERS
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include
    FILES
        include/LfQuick/WidgetBridge/LfQuickWidgetBridgeGlobal.h
        include/LfQuick/WidgetBridge/QuickSurface.h
        include/LfQuick/WidgetBridge/WidgetPageProvider.h
        include/LfQuick/WidgetBridge/WidgetWindowProvider.h
        include/LfQuick/WidgetBridge/WidgetFocusBridge.h
        include/LfQuick/WidgetBridge/WidgetOverlayBackend.h
)
