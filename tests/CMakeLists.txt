find_package(Qt6 6.8.3 REQUIRED COMPONENTS Test)

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

qt_add_executable(tst_runtime_cache
    unit/tst_runtime_cache.cpp
)
target_link_libraries(tst_runtime_cache PRIVATE LfQuick::Runtime Qt6::Test)
lfquick_register_test(tst_runtime_cache)

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

    qt_add_executable(tst_ui_state
        unit/tst_ui_state.cpp
    )
    target_link_libraries(tst_ui_state PRIVATE LfQuick::Ui Qt6::Test)
    lfquick_register_test(tst_ui_state)

    qt_add_executable(tst_navigation_guard_policy
        unit/tst_navigation_guard_policy.cpp
    )
    target_link_libraries(tst_navigation_guard_policy PRIVATE LfQuick::Ui Qt6::Test)
    lfquick_register_test(tst_navigation_guard_policy)
endif()

if(LFQUICK_BUILD_WIDGET_BRIDGE)
    qt_add_executable(tst_widgetbridge
        unit/tst_widgetbridge.cpp
    )
    target_link_libraries(tst_widgetbridge PRIVATE LfQuick::WidgetBridge Qt6::Test)
    lfquick_register_test(tst_widgetbridge)
endif()
