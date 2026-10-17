cmake_minimum_required(VERSION 3.24)

project(LfQuick
    VERSION 0.1.0
    DESCRIPTION "Modular Qt UI Runtime Framework"
    LANGUAGES CXX
)

include(GNUInstallDirs)
include(CTest)

# Keep the project-wide linkage mode explicit. Without this option, ordinary
# add_library() defaults to static while Qt's qt_add_library()/QML APIs can
# follow the Qt build type, producing a mixed static/shared SDK.
option(BUILD_SHARED_LIBS "Build LfQuick libraries as shared libraries" OFF)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(LfQuickOptions)
include(LfQuickVersion)
include(LfQuickInstall)

if(NOT BUILD_SHARED_LIBS)
    add_compile_definitions(LFQUICK_STATIC)
endif()

find_package(Qt6 6.8.3 REQUIRED COMPONENTS Core)
# Qt 6.7+ only honors BUILD_SHARED_LIBS in qt_add_library(), qt_add_plugin()
# and qt_add_qml_module() when QTP0003 uses NEW semantics. LfQuick supports
# both linkage modes, so all module families must follow the same switch.
qt_policy(SET QTP0003 NEW)
include(LfQuickQtFeatures)

if(LFQUICK_BUILD_PLATFORM OR LFQUICK_BUILD_UI OR LFQUICK_BUILD_INTEGRATION
   OR LFQUICK_BUILD_MEDICAL_DEMO OR LFQUICK_BUILD_SCREENSHOT_TOOL
   OR LFQUICK_BUILD_FLUENT_STYLE OR LFQUICK_BUILD_BRAND_TEST_STYLE
   OR LFQUICK_BUILD_LEGACY_QCUSTOMPLOT_ADAPTER)
    find_package(Qt6 6.8.3 REQUIRED COMPONENTS Gui)
endif()

if(LFQUICK_BUILD_PLATFORM_QML OR LFQUICK_BUILD_MODELS OR LFQUICK_BUILD_IO)
    find_package(Qt6 6.8.3 REQUIRED COMPONENTS Qml QmlIntegration)
endif()

if(LFQUICK_BUILD_IO)
    find_package(Qt6 6.8.3 REQUIRED COMPONENTS Network)
endif()

if(LFQUICK_BUILD_UI OR LFQUICK_BUILD_INTEGRATION OR LFQUICK_BUILD_MEDICAL_DEMO
   OR LFQUICK_BUILD_DATAVIEW OR LFQUICK_BUILD_DIAGNOSTICS
   OR LFQUICK_ENABLE_HOT_RELOAD OR LFQUICK_EXPERIMENTAL_STYLEKIT
   OR LFQUICK_BUILD_SCREENSHOT_TOOL OR LFQUICK_BUILD_SAFEAREA_ADAPTER
   OR LFQUICK_BUILD_FLUENT_STYLE OR LFQUICK_BUILD_BRAND_TEST_STYLE
   OR LFQUICK_BUILD_CUSTOM_QUICK_CHARTS)
    find_package(Qt6 6.8.3 REQUIRED COMPONENTS Qml QmlIntegration Quick)
endif()

if(LFQUICK_BUILD_UI OR LFQUICK_BUILD_MEDICAL_DEMO OR LFQUICK_BUILD_DATAVIEW
   OR LFQUICK_BUILD_FLUENT_STYLE OR LFQUICK_BUILD_BRAND_TEST_STYLE)
    find_package(Qt6 6.8.3 REQUIRED COMPONENTS QuickControls2 QuickLayouts)
endif()

if(LFQUICK_BUILD_WIDGET_BRIDGE OR LFQUICK_EXPERIMENTAL_STYLEKIT
   OR LFQUICK_BUILD_LEGACY_QCUSTOMPLOT_ADAPTER)
    find_package(Qt6 6.8.3 REQUIRED COMPONENTS Widgets)
endif()

if(LFQUICK_BUILD_WIDGET_BRIDGE)
    find_package(Qt6 6.8.3 REQUIRED COMPONENTS QuickWidgets)
endif()

if(LFQUICK_BUILD_QT_GRAPHS_ADAPTER)
    if(NOT LFQUICK_BUILD_CHARTS)
        message(FATAL_ERROR "LFQUICK_BUILD_QT_GRAPHS_ADAPTER requires LFQUICK_BUILD_CHARTS=ON")
    endif()
    find_package(Qt6 6.8.3 REQUIRED COMPONENTS Graphs Qml QmlIntegration Quick)
endif()

if(LFQUICK_BUILD_CUSTOM_QUICK_CHARTS AND NOT LFQUICK_BUILD_CHARTS)
    message(FATAL_ERROR "LFQUICK_BUILD_CUSTOM_QUICK_CHARTS requires LFQUICK_BUILD_CHARTS=ON")
endif()

if(LFQUICK_BUILD_LEGACY_QCUSTOMPLOT_ADAPTER AND NOT LFQUICK_BUILD_CHARTS)
    message(FATAL_ERROR "LFQUICK_BUILD_LEGACY_QCUSTOMPLOT_ADAPTER requires LFQUICK_BUILD_CHARTS=ON")
endif()

if(LFQUICK_BUILD_SAFEAREA_ADAPTER)
    lfquick_require_qt_feature("LFQUICK_BUILD_SAFEAREA_ADAPTER" LFQUICK_HAS_SAFE_AREA "6.9.0")
    find_package(Qt6 6.9 REQUIRED COMPONENTS Quick QuickControls2 Qml QmlIntegration)
endif()

if(LFQUICK_BUILD_QRANGE_MODEL_ADAPTER)
    lfquick_require_qt_feature("LFQUICK_BUILD_QRANGE_MODEL_ADAPTER" LFQUICK_HAS_QRANGE_MODEL "6.10.0")
endif()

if(LFQUICK_EXPERIMENTAL_CANVAS_PAINTER)
    lfquick_require_qt_feature("LFQUICK_EXPERIMENTAL_CANVAS_PAINTER" LFQUICK_HAS_CANVAS2D_QML "6.12.0")
    find_package(Qt6 6.12 REQUIRED COMPONENTS CanvasPainter Qml QmlIntegration Quick)
endif()

if(LFQUICK_EXPERIMENTAL_STYLEKIT)
    lfquick_require_qt_feature("LFQUICK_EXPERIMENTAL_STYLEKIT" LFQUICK_HAS_STYLEKIT "6.12.0")
    find_package(Qt6 6.12 REQUIRED COMPONENTS LabsStyleKit)
endif()

if(LFQUICK_BUILD_PLATFORM_QML AND NOT LFQUICK_BUILD_PLATFORM)
    message(FATAL_ERROR "LFQUICK_BUILD_PLATFORM_QML requires LFQUICK_BUILD_PLATFORM=ON")
endif()

if(LFQUICK_BUILD_PLATFORM AND NOT LFQUICK_BUILD_IO)
    message(FATAL_ERROR "LFQUICK_BUILD_PLATFORM requires LFQUICK_BUILD_IO=ON for the FileWatcher compatibility API")
endif()

if(LFQUICK_BUILD_INTEGRATION AND (NOT LFQUICK_BUILD_UI OR NOT LFQUICK_BUILD_PLATFORM))
    message(FATAL_ERROR "LFQUICK_BUILD_INTEGRATION requires LFQUICK_BUILD_UI=ON and LFQUICK_BUILD_PLATFORM=ON")
endif()

add_subdirectory(src/core)
add_subdirectory(src/runtime)

if(LFQUICK_BUILD_MODELS)
    add_subdirectory(src/models)
endif()

if(LFQUICK_BUILD_IO)
    add_subdirectory(src/io)
endif()

if(LFQUICK_BUILD_PLATFORM)
    add_subdirectory(src/platform)
endif()

if(LFQUICK_BUILD_PLATFORM_QML)
    add_subdirectory(src/platformqml)
endif()

if(LFQUICK_BUILD_DATAVIEW AND NOT LFQUICK_BUILD_UI)
    message(FATAL_ERROR "LFQUICK_BUILD_DATAVIEW requires LFQUICK_BUILD_UI=ON")
endif()

if(LFQUICK_BUILD_WIDGET_BRIDGE AND NOT LFQUICK_BUILD_UI)
    message(FATAL_ERROR "LFQUICK_BUILD_WIDGET_BRIDGE requires LFQUICK_BUILD_UI=ON")
endif()

if(LFQUICK_ENABLE_HOT_RELOAD AND NOT LFQUICK_BUILD_UI)
    message(FATAL_ERROR "LFQUICK_ENABLE_HOT_RELOAD requires LFQUICK_BUILD_UI=ON")
endif()

if(LFQUICK_BUILD_HYBRID_DEMO AND NOT LFQUICK_BUILD_WIDGET_BRIDGE)
    message(FATAL_ERROR "LFQUICK_BUILD_HYBRID_DEMO requires LFQUICK_BUILD_WIDGET_BRIDGE=ON")
endif()

if(LFQUICK_BUILD_MEDICAL_DEMO AND (NOT LFQUICK_BUILD_UI OR NOT LFQUICK_BUILD_DATAVIEW OR NOT LFQUICK_BUILD_PLATFORM))
    message(FATAL_ERROR "LFQUICK_BUILD_MEDICAL_DEMO requires UI, DataView and Platform modules")
endif()

if(LFQUICK_BUILD_UI)
    add_subdirectory(src/ui)
endif()

if(LFQUICK_BUILD_INTEGRATION)
    add_subdirectory(src/integration)
endif()

if(LFQUICK_BUILD_FLUENT_STYLE)
    add_subdirectory(src/fluentstyle)
endif()

if(LFQUICK_BUILD_BRAND_TEST_STYLE)
    add_subdirectory(src/brandtest)
endif()

if(LFQUICK_ENABLE_HOT_RELOAD)
    add_subdirectory(src/tooling)
endif()

if(LFQUICK_EXPERIMENTAL_STYLEKIT)
    add_subdirectory(src/stylekit)
endif()

if(LFQUICK_BUILD_SAFEAREA_ADAPTER)
    add_subdirectory(src/qtadapters/safearea)
endif()

if(LFQUICK_BUILD_QRANGE_MODEL_ADAPTER)
    add_subdirectory(src/qtadapters/rangemodel)
endif()

if(LFQUICK_EXPERIMENTAL_CANVAS_PAINTER)
    add_subdirectory(src/qtadapters/canvaspainter)
endif()

if(LFQUICK_BUILD_DATAVIEW)
    add_subdirectory(src/dataview)
endif()

if(LFQUICK_BUILD_WIDGET_BRIDGE)
    add_subdirectory(src/widgetbridge)
endif()

if(LFQUICK_BUILD_CHARTS)
    add_subdirectory(src/charts)
endif()

if(LFQUICK_BUILD_CUSTOM_QUICK_CHARTS)
    add_subdirectory(src/qtadapters/customquickcharts)
endif()

if(LFQUICK_BUILD_LEGACY_QCUSTOMPLOT_ADAPTER)
    add_subdirectory(src/qtadapters/qcustomplot)
endif()

if(LFQUICK_BUILD_DIAGNOSTICS)
    add_subdirectory(src/diagnostics)
endif()

# Install/export the framework libraries using their public alias names.
lfquick_install_target(LfQuickCore Core)
lfquick_install_target(LfQuickRuntime Runtime)
lfquick_install_target(LfQuickModels Models)
lfquick_install_target(LfQuickIo Io)
lfquick_install_target(LfQuickPlatform Platform)
lfquick_install_target(LfQuickPlatformQml PlatformQml)
lfquick_install_target(LfQuickUi Ui)
lfquick_install_target(LfQuickIntegration Integration)
lfquick_install_target(LfQuickDataView DataView)
lfquick_install_target(LfQuickWidgetBridge WidgetBridge)
lfquick_install_target(LfQuickCharts Charts)
lfquick_install_target(LfQuickDiagnostics Diagnostics)
lfquick_install_target(LfQuickTooling Tooling)
lfquick_install_target(LfQuickStyleKit StyleKit)
lfquick_install_target(LfQuickSafeAreaAdapter SafeAreaAdapter)
lfquick_install_target(LfQuickRangeModelAdapter RangeModelAdapter)
lfquick_install_target(LfQuickCanvasPainterAdapter CanvasPainterAdapter)
lfquick_install_target(LfQuickCustomQuickCharts CustomQuickCharts)
lfquick_install_target(LfQuickLegacyQCustomPlotAdapter LegacyQCustomPlotAdapter)
lfquick_install_target(LfQuickFluentStyle FluentStyle)
lfquick_install_target(LfQuickBrandTestStyle BrandTestStyle)

set(LFQUICK_PACKAGE_QT_COMPONENTS Core)
if(TARGET LfQuickModels)
    list(APPEND LFQUICK_PACKAGE_QT_COMPONENTS Qml QmlIntegration)
endif()
if(TARGET LfQuickIo)
    list(APPEND LFQUICK_PACKAGE_QT_COMPONENTS Qml QmlIntegration Network)
endif()
if(TARGET LfQuickPlatform)
    list(APPEND LFQUICK_PACKAGE_QT_COMPONENTS Gui)
endif()
if(TARGET LfQuickPlatformQml)
    list(APPEND LFQUICK_PACKAGE_QT_COMPONENTS Qml QmlIntegration)
endif()
if(TARGET LfQuickUi OR TARGET LfQuickIntegration OR TARGET LfQuickDataView
   OR TARGET LfQuickDiagnostics OR TARGET LfQuickTooling
   OR TARGET LfQuickSafeAreaAdapter OR TARGET LfQuickCustomQuickCharts
   OR TARGET LfQuickFluentStyle OR TARGET LfQuickBrandTestStyle)
    list(APPEND LFQUICK_PACKAGE_QT_COMPONENTS Gui Qml QmlIntegration Quick)
endif()
if(TARGET LfQuickUi OR TARGET LfQuickDataView OR TARGET LfQuickSafeAreaAdapter
   OR TARGET LfQuickFluentStyle OR TARGET LfQuickBrandTestStyle)
    list(APPEND LFQUICK_PACKAGE_QT_COMPONENTS QuickControls2 QuickLayouts)
endif()
if(TARGET LfQuickWidgetBridge)
    list(APPEND LFQUICK_PACKAGE_QT_COMPONENTS Widgets QuickWidgets)
endif()
if(TARGET LfQuickLegacyQCustomPlotAdapter)
    list(APPEND LFQUICK_PACKAGE_QT_COMPONENTS Widgets)
endif()
if(LFQUICK_BUILD_QT_GRAPHS_ADAPTER)
    # LfQuickCharts exports these Qt targets in its public link interface when
    # the Graphs-backed QML adapter is compiled in. Mirror the build-time
    # find_package() contract so an installed LfQuickTargets.cmake never
    # references missing Qt imported targets in downstream projects.
    list(APPEND LFQUICK_PACKAGE_QT_COMPONENTS Graphs Qml QmlIntegration Quick)
endif()
if(TARGET LfQuickCanvasPainterAdapter)
    list(APPEND LFQUICK_PACKAGE_QT_COMPONENTS CanvasPainter Qml QmlIntegration Quick)
endif()
if(TARGET LfQuickStyleKit)
    list(APPEND LFQUICK_PACKAGE_QT_COMPONENTS Qml QmlIntegration Widgets LabsStyleKit)
endif()
list(REMOVE_DUPLICATES LFQUICK_PACKAGE_QT_COMPONENTS)
lfquick_install_package()

if(LFQUICK_BUILD_SCREENSHOT_TOOL)
    add_subdirectory(tools/screenshot)
endif()

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

if(LFQUICK_BUILD_MEDICAL_DEMO)
    add_subdirectory(examples/medical-demo)
endif()

if(LFQUICK_BUILD_HYBRID_DEMO)
    add_subdirectory(examples/hybrid-demo)
endif()
