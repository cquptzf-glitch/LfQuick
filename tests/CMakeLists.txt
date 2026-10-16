find_package(Qt6 6.8.3 REQUIRED COMPONENTS Test)

function(lfquick_configure_test_runtime test_name)
    if(WIN32 AND BUILD_SHARED_LIBS)
        set(_runtime_targets
            Qt6::Core
            LfQuickCore
            LfQuickRuntime
        )

        foreach(_optional_target IN ITEMS
                LfQuickUi
                LfQuickDataView
                LfQuickPlatform
                LfQuickWidgetBridge)
            if(TARGET ${_optional_target})
                list(APPEND _runtime_targets ${_optional_target})
            endif()
        endforeach()

        set(_runtime_environment)
        foreach(_runtime_target IN LISTS _runtime_targets)
            list(APPEND _runtime_environment
                "PATH=path_list_prepend:$<TARGET_FILE_DIR:${_runtime_target}>")
        endforeach()

        set_tests_properties(${test_name}
            PROPERTIES
                ENVIRONMENT_MODIFICATION "${_runtime_environment}"
        )
    endif()
endfunction()

function(lfquick_register_test test_name)
    add_test(NAME ${test_name} COMMAND ${test_name})
    lfquick_configure_test_runtime(${test_name})
endfunction()

qt_add_executable(tst_core
    unit/tst_core.cpp
)
target_link_libraries(tst_core PRIVATE LfQuick::Core Qt6::Test)
lfquick_register_test(tst_core)

qt_add_executable(tst_runtime
    unit/tst_runtime.cpp
)
target_link_libraries(tst_runtime PRIVATE LfQuick::Runtime Qt6::Test)
lfquick_register_test(tst_runtime)

qt_add_executable(tst_runtime_abi
    unit/tst_runtime_abi.cpp
)
target_link_libraries(tst_runtime_abi PRIVATE LfQuick::Runtime Qt6::Test)
lfquick_register_test(tst_runtime_abi)

if(LFQUICK_BUILD_DATAVIEW)
    qt_add_executable(tst_dataview
        unit/tst_dataview.cpp
    )
    target_link_libraries(tst_dataview PRIVATE LfQuick::DataView Qt6::Test)
    lfquick_register_test(tst_dataview)

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
    lfquick_register_test(tst_platform)
endif()

if(LFQUICK_BUILD_UI)
    qt_add_executable(tst_ui
        unit/tst_ui.cpp
    )
    target_link_libraries(tst_ui PRIVATE LfQuick::Ui Qt6::Test)
    lfquick_register_test(tst_ui)
endif()

if(LFQUICK_BUILD_WIDGET_BRIDGE)
    qt_add_executable(tst_widgetbridge
        unit/tst_widgetbridge.cpp
    )
    target_link_libraries(tst_widgetbridge PRIVATE LfQuick::WidgetBridge Qt6::Test)
    lfquick_register_test(tst_widgetbridge)
endif()
