cmake_minimum_required(VERSION 3.24)

project(LfQuick
    VERSION 0.1.0
    DESCRIPTION "Modular Qt UI Runtime Framework"
    LANGUAGES CXX
)

include(GNUInstallDirs)
include(CTest)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(LfQuickOptions)
include(LfQuickVersion)
include(LfQuickQtFeatures)

if(NOT BUILD_SHARED_LIBS)
    add_compile_definitions(LFQUICK_STATIC)
endif()

find_package(Qt6 6.8.3 REQUIRED COMPONENTS Core)

if(LFQUICK_BUILD_PLATFORM OR LFQUICK_BUILD_UI OR LFQUICK_BUILD_MEDICAL_DEMO)
    find_package(Qt6 6.8.3 REQUIRED COMPONENTS Gui)
endif()

if(LFQUICK_BUILD_UI OR LFQUICK_BUILD_MEDICAL_DEMO OR LFQUICK_BUILD_DATAVIEW
   OR LFQUICK_BUILD_DIAGNOSTICS OR LFQUICK_ENABLE_HOT_RELOAD
   OR LFQUICK_EXPERIMENTAL_STYLEKIT)
    find_package(Qt6 6.8.3 REQUIRED COMPONENTS Qml QmlIntegration Quick)
endif()

if(LFQUICK_BUILD_UI OR LFQUICK_BUILD_MEDICAL_DEMO OR LFQUICK_BUILD_DATAVIEW)
    find_package(Qt6 6.8.3 REQUIRED COMPONENTS QuickControls2 QuickLayouts)
endif()

if(LFQUICK_BUILD_WIDGET_BRIDGE OR LFQUICK_EXPERIMENTAL_STYLEKIT)
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

if(LFQUICK_EXPERIMENTAL_STYLEKIT)
    lfquick_require_qt_feature("LFQUICK_EXPERIMENTAL_STYLEKIT" LFQUICK_HAS_STYLEKIT "6.12.0")
    find_package(Qt6 6.12 REQUIRED COMPONENTS LabsStyleKit)
endif()

add_subdirectory(src/core)
add_subdirectory(src/runtime)

if(LFQUICK_BUILD_PLATFORM)
    add_subdirectory(src/platform)
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

if(LFQUICK_ENABLE_HOT_RELOAD)
    add_subdirectory(src/tooling)
endif()

if(LFQUICK_EXPERIMENTAL_STYLEKIT)
    add_subdirectory(src/stylekit)
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

if(LFQUICK_BUILD_DIAGNOSTICS)
    add_subdirectory(src/diagnostics)
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
