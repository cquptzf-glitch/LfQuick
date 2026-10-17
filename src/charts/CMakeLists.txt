add_library(LfQuickCharts
    src/ChartDataModel.cpp
    src/SeriesModel.cpp
    src/ChartBackendRegistry.cpp
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
        include/LfQuick/Charts/ChartBackendRegistry.h
)

if(LFQUICK_BUILD_QT_GRAPHS_ADAPTER)
    target_sources(LfQuickCharts
        PRIVATE
            src/QtGraphsSeriesAdapter.cpp
            src/QtGraphsAxisAdapter.cpp
        PUBLIC
            FILE_SET HEADERS
            BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include
            FILES
                include/LfQuick/Charts/QtGraphsSeriesAdapter.h
                include/LfQuick/Charts/QtGraphsAxisAdapter.h
    )
    target_link_libraries(LfQuickCharts PUBLIC Qt6::Graphs Qt6::Qml Qt6::Quick)
    target_compile_definitions(LfQuickCharts PUBLIC LFQUICK_HAS_QT_GRAPHS_ADAPTER=1)

    qt_add_qml_module(LfQuickCharts
        URI LfQuick.Charts
        VERSION 1.0
        RESOURCE_PREFIX /qt/qml
        QML_FILES
            qml/ChartView.qml
    )
endif()
