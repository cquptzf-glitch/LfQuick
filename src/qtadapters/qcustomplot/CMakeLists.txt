add_library(LfQuickLegacyQCustomPlotAdapter
    src/LegacyQCustomPlotBackend.cpp
)
add_library(LfQuick::LegacyQCustomPlotAdapter ALIAS LfQuickLegacyQCustomPlotAdapter)

target_compile_definitions(LfQuickLegacyQCustomPlotAdapter PRIVATE LFQUICKLEGACYQCP_LIBRARY)

target_include_directories(LfQuickLegacyQCustomPlotAdapter
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_sources(LfQuickLegacyQCustomPlotAdapter PUBLIC
    FILE_SET HEADERS
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include
    FILES
        include/LfQuick/QtAdapters/LfQuickLegacyQCustomPlotGlobal.h
        include/LfQuick/QtAdapters/LegacyQCustomPlotBackend.h
)

target_link_libraries(LfQuickLegacyQCustomPlotAdapter
    PUBLIC
        LfQuick::Charts
        Qt6::Core
        Qt6::Widgets
)
