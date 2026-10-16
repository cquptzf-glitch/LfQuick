add_library(LfQuickCharts
    src/ChartDataModel.cpp
    src/SeriesModel.cpp
)
add_library(LfQuick::Charts ALIAS LfQuickCharts)

target_compile_definitions(LfQuickCharts PRIVATE LFQUICKCHARTS_LIBRARY)

target_include_directories(LfQuickCharts
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_link_libraries(LfQuickCharts PUBLIC Qt6::Core)

target_sources(LfQuickCharts PUBLIC
    FILE_SET HEADERS
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include
    FILES
        include/LfQuick/Charts/LfQuickChartsGlobal.h
        include/LfQuick/Charts/ChartDescriptor.h
        include/LfQuick/Charts/ChartDataModel.h
        include/LfQuick/Charts/SeriesModel.h
)
