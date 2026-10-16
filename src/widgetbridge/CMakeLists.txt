
add_library(LfQuickWidgetBridge
    src/QuickSurface.cpp
)
add_library(LfQuick::WidgetBridge ALIAS LfQuickWidgetBridge)

target_include_directories(LfQuickWidgetBridge
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
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
    FILES include/LfQuick/WidgetBridge/QuickSurface.h
)
