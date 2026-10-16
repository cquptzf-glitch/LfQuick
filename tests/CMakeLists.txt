find_package(Qt6 6.8.3 REQUIRED COMPONENTS Test)

qt_add_executable(tst_core
    unit/tst_core.cpp
)
target_link_libraries(tst_core PRIVATE LfQuick::Core Qt6::Test)
add_test(NAME tst_core COMMAND tst_core)

qt_add_executable(tst_runtime
    unit/tst_runtime.cpp
)
target_link_libraries(tst_runtime PRIVATE LfQuick::Runtime Qt6::Test)
add_test(NAME tst_runtime COMMAND tst_runtime)

if(LFQUICK_BUILD_DATAVIEW)
    qt_add_executable(tst_dataview
        unit/tst_dataview.cpp
    )
    target_link_libraries(tst_dataview PRIVATE LfQuick::DataView Qt6::Test)
    add_test(NAME tst_dataview COMMAND tst_dataview)

    if(LFQUICK_BUILD_BENCHMARKS)
        qt_add_executable(lfquick_dataview_benchmark
            performance/dataview_benchmark.cpp
        )
        target_link_libraries(lfquick_dataview_benchmark PRIVATE LfQuick::DataView Qt6::Core)
    endif()
endif()

if(LFQUICK_BUILD_PLATFORM)
    qt_add_executable(tst_platform
        unit/tst_platform.cpp
    )
    target_link_libraries(tst_platform PRIVATE LfQuick::Platform Qt6::Test)
    add_test(NAME tst_platform COMMAND tst_platform)
endif()

if(LFQUICK_BUILD_UI)
    qt_add_executable(tst_ui
        unit/tst_ui.cpp
    )
    target_link_libraries(tst_ui PRIVATE LfQuick::Ui Qt6::Test)
    add_test(NAME tst_ui COMMAND tst_ui)
endif()

if(LFQUICK_BUILD_WIDGET_BRIDGE)
    qt_add_executable(tst_widgetbridge
        unit/tst_widgetbridge.cpp
    )
    target_link_libraries(tst_widgetbridge PRIVATE LfQuick::WidgetBridge Qt6::Test)
    add_test(NAME tst_widgetbridge COMMAND tst_widgetbridge)
endif()
