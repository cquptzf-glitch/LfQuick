find_package(Qt6 6.8.3 REQUIRED COMPONENTS Test)
find_package(Python3 COMPONENTS Interpreter QUIET)

function(lfquick_configure_test_runtime test_name)
    if(WIN32)
        # Make every test directly runnable from its build output directory.
        # TARGET_RUNTIME_DLLS follows the target's transitive shared-library
        # dependencies (LfQuick modules and Qt) and avoids relying on a CI-only
        # PATH mutation.
        add_custom_command(TARGET ${test_name} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                $<TARGET_RUNTIME_DLLS:${test_name}>
                $<TARGET_FILE_DIR:${test_name}>
            COMMAND_EXPAND_LISTS
            VERBATIM
        )
    endif()
endfunction()

function(lfquick_register_test test_name)
    add_test(NAME ${test_name} COMMAND ${test_name})
    lfquick_configure_test_runtime(${test_name})
endfunction()

if(Python3_Interpreter_FOUND)
    add_test(
        NAME tooling_python_selftest
        COMMAND ${Python3_EXECUTABLE} ${PROJECT_SOURCE_DIR}/tools/test_tooling.py
    )
endif()

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

qt_add_executable(tst_runtime_stress
    unit/tst_runtime_stress.cpp
)
target_link_libraries(tst_runtime_stress PRIVATE LfQuick::Runtime Qt6::Test)
lfquick_register_test(tst_runtime_stress)

qt_add_executable(tst_runtime_abi
    unit/tst_runtime_abi.cpp
)
target_link_libraries(tst_runtime_abi PRIVATE LfQuick::Runtime Qt6::Test)
lfquick_register_test(tst_runtime_abi)

qt_add_executable(tst_runtime_cache
    unit/tst_runtime_cache.cpp
)
target_link_libraries(tst_runtime_cache PRIVATE LfQuick::Runtime Qt6::Test)
lfquick_register_test(tst_runtime_cache)

qt_add_executable(tst_window_router
    unit/tst_window_router.cpp
)
target_link_libraries(tst_window_router PRIVATE LfQuick::Runtime Qt6::Test)
lfquick_register_test(tst_window_router)

qt_add_executable(tst_window_owner
    unit/tst_window_owner.cpp
)
target_link_libraries(tst_window_owner PRIVATE LfQuick::Runtime Qt6::Test)
lfquick_register_test(tst_window_owner)

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
        if(WIN32)
            target_link_libraries(lfquick_dataview_benchmark PRIVATE Psapi)
        endif()
    endif()
endif()

if(LFQUICK_BUILD_PLATFORM)
    qt_add_executable(tst_platform
        unit/tst_platform.cpp
    )
    target_link_libraries(tst_platform PRIVATE LfQuick::Platform Qt6::Test)
    lfquick_register_test(tst_platform)

    qt_add_executable(tst_scanner_stress
        unit/tst_scanner_stress.cpp
    )
    target_link_libraries(tst_scanner_stress PRIVATE LfQuick::Platform Qt6::Test)
    lfquick_register_test(tst_scanner_stress)
endif()

if(LFQUICK_BUILD_UI)
    qt_add_executable(tst_ui
        unit/tst_ui.cpp
    )
    target_link_libraries(tst_ui PRIVATE LfQuick::Ui Qt6::Test)
    lfquick_register_test(tst_ui)

    qt_add_executable(tst_ui_state
        unit/tst_ui_state.cpp
    )
    target_link_libraries(tst_ui_state PRIVATE LfQuick::Ui Qt6::Test)
    lfquick_register_test(tst_ui_state)

    qt_add_executable(tst_ui_controls
        unit/tst_ui_controls.cpp
    )
    target_link_libraries(tst_ui_controls PRIVATE LfQuick::Ui Qt6::Test)
    lfquick_register_test(tst_ui_controls)

    qt_add_executable(tst_focus_lifecycle
        unit/tst_focus_lifecycle.cpp
    )
    target_link_libraries(tst_focus_lifecycle PRIVATE LfQuick::Ui Qt6::Test)
    lfquick_register_test(tst_focus_lifecycle)

    qt_add_executable(tst_busy_token
        unit/tst_busy_token.cpp
    )
    target_link_libraries(tst_busy_token PRIVATE LfQuick::Ui Qt6::Test)
    lfquick_register_test(tst_busy_token)

    qt_add_executable(tst_navigation_guard_policy
        unit/tst_navigation_guard_policy.cpp
    )
    target_link_libraries(tst_navigation_guard_policy PRIVATE LfQuick::Ui Qt6::Test)
    lfquick_register_test(tst_navigation_guard_policy)

    qt_add_executable(tst_navigation_model
        unit/tst_navigation_model.cpp
    )
    target_link_libraries(tst_navigation_model PRIVATE LfQuick::Ui Qt6::Test)
    lfquick_register_test(tst_navigation_model)
endif()

if(LFQUICK_BUILD_INTEGRATION)
    qt_add_executable(tst_integration
        unit/tst_integration.cpp
    )
    target_link_libraries(tst_integration PRIVATE LfQuick::Integration Qt6::Test)
    lfquick_register_test(tst_integration)
endif()

if(LFQUICK_BUILD_WIDGET_BRIDGE)
    qt_add_executable(tst_widgetbridge
        unit/tst_widgetbridge.cpp
    )
    target_link_libraries(tst_widgetbridge PRIVATE LfQuick::WidgetBridge Qt6::Test)
    lfquick_register_test(tst_widgetbridge)

    qt_add_executable(tst_widgetbridge_stress
        unit/tst_widgetbridge_stress.cpp
    )
    target_link_libraries(tst_widgetbridge_stress PRIVATE LfQuick::WidgetBridge Qt6::Test)
    lfquick_register_test(tst_widgetbridge_stress)
endif()

if(LFQUICK_BUILD_CHARTS)
    qt_add_executable(tst_charts
        unit/tst_charts.cpp
    )
    target_link_libraries(tst_charts PRIVATE LfQuick::Charts Qt6::Test)
    lfquick_register_test(tst_charts)
endif()

if(LFQUICK_BUILD_DIAGNOSTICS)
    qt_add_executable(tst_diagnostics
        unit/tst_diagnostics.cpp
    )
    target_link_libraries(tst_diagnostics PRIVATE LfQuick::Diagnostics Qt6::Test)
    lfquick_register_test(tst_diagnostics)
endif()

if(LFQUICK_ENABLE_HOT_RELOAD)
    qt_add_executable(tst_tooling
        unit/tst_tooling.cpp
    )
    target_link_libraries(tst_tooling PRIVATE LfQuick::Tooling Qt6::Test)
    lfquick_register_test(tst_tooling)
endif()

if(LFQUICK_BUILD_QRANGE_MODEL_ADAPTER)
    qt_add_executable(tst_range_model_adapter
        unit/tst_range_model_adapter.cpp
    )
    target_link_libraries(tst_range_model_adapter PRIVATE LfQuick::RangeModelAdapter Qt6::Test)
    lfquick_register_test(tst_range_model_adapter)
endif()
