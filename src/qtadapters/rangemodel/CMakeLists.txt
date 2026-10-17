qt_add_library(LfQuickRangeModelAdapter
    src/RangeModelBridge.cpp
)
add_library(LfQuick::RangeModelAdapter ALIAS LfQuickRangeModelAdapter)

target_compile_definitions(LfQuickRangeModelAdapter PRIVATE LFQUICKRANGEMODELADAPTER_LIBRARY)

target_include_directories(LfQuickRangeModelAdapter
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_sources(LfQuickRangeModelAdapter PUBLIC
    FILE_SET HEADERS
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include
    FILES
        include/LfQuick/QtAdapters/LfQuickRangeModelAdapterGlobal.h
        include/LfQuick/QtAdapters/RangeModelBridge.h
)

target_link_libraries(LfQuickRangeModelAdapter
    PUBLIC
        Qt6::Core
)
