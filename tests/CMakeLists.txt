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

qt_add_executable(tst_window_reload
    unit/tst_window_reload.cpp
)
target_link_libraries(tst_window_reload PRIVATE LfQuick::Runtime Qt6::Test)
lfquick_register_test(tst_window_reload)

qt_add_executable(tst_window_owner
    unit/tst_window_owner.cpp
)
target_link_libraries(tst_window_owner PRIVATE LfQuick::Runtime Qt6::Test)
lfquick_register_test(tst_window_owner)

if(LFQUICK_BUILD_MODELS)
    qt_add_executable(tst_models
        unit/tst_models.cpp
    )
    target_link_libraries(tst_models PRIVATE LfQuick::Models Qt6::Qml Qt6::Test)
    lfquick_register_test(tst_models)
endif()

if(LFQUICK_BUILD_IO)
    qt_add_executable(tst_io
        unit/tst_io.cpp
    )
    target_link_libraries(tst_io PRIVATE LfQuick::Io Qt6::Test)
    lfquick_register_test(tst_io)
endif()

if(LFQUICK_BUILD_DATAVIEW)
    qt_add_executable(tst_dataview
        unit/tst_dataview.cpp
    )
    target_link_libraries(tst_dataview PRIVATE LfQuick::DataView Qt6::Test)
    lfquick_register_test(tst_dataview)

    qt_add_executable(tst_dataview_qml
        unit/tst_dataview_qml.cpp
    )
    target_link_libraries(tst_dataview_qml PRIVATE LfQuick::DataView Qt6::Qml Qt6::Test)
    lfquick_register_test(tst_dataview_qml)

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

    qt_add_executable(tst_hotkey_router
        unit/tst_hotkey_router.cpp
    )
    target_link_libraries(tst_hotkey_router PRIVATE LfQuick::Platform Qt6::Test)
    lfquick_register_test(tst_hotkey_router)

    qt_add_executable(tst_scanner_stress
        unit/tst_scanner_stress.cpp
    )
    target_link_libraries(tst_scanner_stress PRIVATE LfQuick::Platform Qt6::Test)
    lfquick_register_test(tst_scanner_stress)
endif()

if(LFQUICK_BUILD_PLATFORM_QML)
    qt_add_executable(tst_platform_qml
        unit/tst_platform_qml.cpp
    )
    target_link_libraries(tst_platform_qml PRIVATE LfQuick::PlatformQml Qt6::Qml Qt6::Test)
    lfquick_register_test(tst_platform_qml)
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

    qt_add_executable(tst_quick_page_lifecycle
        unit/tst_quick_page_lifecycle.cpp
    )
    target_link_libraries(tst_quick_page_lifecycle
        PRIVATE LfQuick::Ui Qt6::Qml Qt6::Quick Qt6::Test)
    lfquick_register_test(tst_quick_page_lifecycle)

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

    qt_add_executable(tst_runtime_infrastructure
        unit/tst_runtime_infrastructure.cpp
    )
    target_link_libraries(tst_runtime_infrastructure
        PRIVATE LfQuick::Tooling Qt6::Qml Qt6::Test)
    lfquick_register_test(tst_runtime_infrastructure)
endif()

if(LFQUICK_BUILD_QRANGE_MODEL_ADAPTER)
    qt_add_executable(tst_range_model_adapter
        unit/tst_range_model_adapter.cpp
    )
    target_link_libraries(tst_range_model_adapter PRIVATE LfQuick::RangeModelAdapter Qt6::Test)
    lfquick_register_test(tst_range_model_adapter)
endif()

# Verify the installed SDK rather than only the in-tree aliases. The consumer
# requires every component that exists in this build, so missing exported Qt
# dependencies and broken check_required_components() semantics fail during
# the downstream configure/generate step.
set(LFQUICK_PACKAGE_SMOKE_COMPONENTS Core Runtime)
macro(lfquick_append_package_smoke_component target component)
    if(TARGET ${target})
        list(APPEND LFQUICK_PACKAGE_SMOKE_COMPONENTS ${component})
    endif()
endmacro()

lfquick_append_package_smoke_component(LfQuickModels Models)
lfquick_append_package_smoke_component(LfQuickIo Io)
lfquick_append_package_smoke_component(LfQuickPlatform Platform)
lfquick_append_package_smoke_component(LfQuickPlatformQml PlatformQml)
lfquick_append_package_smoke_component(LfQuickUi Ui)
lfquick_append_package_smoke_component(LfQuickIntegration Integration)
lfquick_append_package_smoke_component(LfQuickDataView DataView)
lfquick_append_package_smoke_component(LfQuickWidgetBridge WidgetBridge)
lfquick_append_package_smoke_component(LfQuickCharts Charts)
lfquick_append_package_smoke_component(LfQuickDiagnostics Diagnostics)
lfquick_append_package_smoke_component(LfQuickTooling Tooling)
lfquick_append_package_smoke_component(LfQuickStyleKit StyleKit)
lfquick_append_package_smoke_component(LfQuickSafeAreaAdapter SafeAreaAdapter)
lfquick_append_package_smoke_component(LfQuickRangeModelAdapter RangeModelAdapter)
lfquick_append_package_smoke_component(LfQuickCanvasPainterAdapter CanvasPainterAdapter)
lfquick_append_package_smoke_component(LfQuickCustomQuickCharts CustomQuickCharts)
lfquick_append_package_smoke_component(LfQuickLegacyQCustomPlotAdapter LegacyQCustomPlotAdapter)
lfquick_append_package_smoke_component(LfQuickFluentStyle FluentStyle)
lfquick_append_package_smoke_component(LfQuickBrandTestStyle BrandTestStyle)
list(REMOVE_DUPLICATES LFQUICK_PACKAGE_SMOKE_COMPONENTS)
list(JOIN LFQUICK_PACKAGE_SMOKE_COMPONENTS " " LFQUICK_PACKAGE_SMOKE_COMPONENTS_ARGS)

# Static QML plugins are link-time SDK artifacts. lfquick_install_target()
# gives each one a stable <Component>QmlPlugin export name; collect the ones
# that exist in this build and require the downstream fixture to link them.
set(LFQUICK_PACKAGE_SMOKE_QML_PLUGINS)
if(NOT BUILD_SHARED_LIBS)
    macro(lfquick_append_package_smoke_qml_plugin backing_target)
        if(TARGET ${backing_target})
            get_target_property(_lfquick_plugin_target
                ${backing_target} QT_QML_MODULE_PLUGIN_TARGET)
            if(_lfquick_plugin_target
               AND NOT _lfquick_plugin_target MATCHES "-NOTFOUND$"
               AND TARGET ${_lfquick_plugin_target})
                get_target_property(_lfquick_plugin_export_name
                    ${_lfquick_plugin_target} EXPORT_NAME)
                if(_lfquick_plugin_export_name
                   AND NOT _lfquick_plugin_export_name MATCHES "-NOTFOUND$")
                    list(APPEND LFQUICK_PACKAGE_SMOKE_QML_PLUGINS
                        ${_lfquick_plugin_export_name})
                endif()
            endif()
        endif()
    endmacro()

    lfquick_append_package_smoke_qml_plugin(LfQuickModels)
    lfquick_append_package_smoke_qml_plugin(LfQuickIo)
    lfquick_append_package_smoke_qml_plugin(LfQuickPlatformQml)
    lfquick_append_package_smoke_qml_plugin(LfQuickUi)
    lfquick_append_package_smoke_qml_plugin(LfQuickIntegration)
    lfquick_append_package_smoke_qml_plugin(LfQuickDataView)
    lfquick_append_package_smoke_qml_plugin(LfQuickCharts)
    lfquick_append_package_smoke_qml_plugin(LfQuickDiagnostics)
    lfquick_append_package_smoke_qml_plugin(LfQuickTooling)
    lfquick_append_package_smoke_qml_plugin(LfQuickStyleKit)
    lfquick_append_package_smoke_qml_plugin(LfQuickSafeAreaAdapter)
    lfquick_append_package_smoke_qml_plugin(LfQuickCanvasPainterAdapter)
    lfquick_append_package_smoke_qml_plugin(LfQuickCustomQuickCharts)
    lfquick_append_package_smoke_qml_plugin(LfQuickFluentStyle)
    lfquick_append_package_smoke_qml_plugin(LfQuickBrandTestStyle)
endif()
list(REMOVE_DUPLICATES LFQUICK_PACKAGE_SMOKE_QML_PLUGINS)
list(JOIN LFQUICK_PACKAGE_SMOKE_QML_PLUGINS " " LFQUICK_PACKAGE_SMOKE_QML_PLUGIN_ARGS)

set(_lfquick_package_consumer_source_dir
    "${CMAKE_CURRENT_BINARY_DIR}/package-consumer-source")
set(_lfquick_package_consumer_build_dir
    "${CMAKE_CURRENT_BINARY_DIR}/package-consumer-build")
set(_lfquick_package_install_prefix
    "${CMAKE_CURRENT_BINARY_DIR}/package-install")
file(MAKE_DIRECTORY "${_lfquick_package_consumer_source_dir}")
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/package/CMakeLists.txt.in"
    "${_lfquick_package_consumer_source_dir}/CMakeLists.txt"
    @ONLY
)
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/package/main.cpp"
    "${_lfquick_package_consumer_source_dir}/main.cpp"
    COPYONLY
)

add_test(
    NAME package_downstream_smoke
    COMMAND ${CMAKE_COMMAND}
        "-DLFQUICK_BUILD_DIR=${PROJECT_BINARY_DIR}"
        "-DLFQUICK_INSTALL_PREFIX=${_lfquick_package_install_prefix}"
        "-DLFQUICK_CONSUMER_SOURCE_DIR=${_lfquick_package_consumer_source_dir}"
        "-DLFQUICK_CONSUMER_BUILD_DIR=${_lfquick_package_consumer_build_dir}"
        "-DLFQUICK_QT6_DIR=${Qt6_DIR}"
        "-DLFQUICK_CONFIG=$<CONFIG>"
        "-DLFQUICK_GENERATOR=${CMAKE_GENERATOR}"
        "-DLFQUICK_GENERATOR_PLATFORM=${CMAKE_GENERATOR_PLATFORM}"
        "-DLFQUICK_GENERATOR_TOOLSET=${CMAKE_GENERATOR_TOOLSET}"
        -P "${CMAKE_CURRENT_SOURCE_DIR}/package/run_downstream_smoke.cmake"
)
