
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

if(NOT BUILD_SHARED_LIBS)
    add_compile_definitions(LFQUICK_STATIC)
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(LfQuickOptions)
include(LfQuickVersion)
include(LfQuickQtFeatures)

find_package(Qt6 6.8.3 REQUIRED COMPONENTS Core)

if(LFQUICK_BUILD_PLATFORM OR LFQUICK_BUILD_UI OR LFQUICK_BUILD_MEDICAL_DEMO)
    find_package(Qt6 6.8.3 REQUIRED COMPONENTS Gui Qml Quick QuickControls2 QuickLayouts)
endif()

if(LFQUICK_BUILD_WIDGET_BRIDGE)
    find_package(Qt6 6.8.3 REQUIRED COMPONENTS Widgets QuickWidgets)
endif()

add_subdirectory(src/core)
add_subdirectory(src/runtime)

if(LFQUICK_BUILD_PLATFORM)
    add_subdirectory(src/platform)
endif()

if(LFQUICK_BUILD_DATAVIEW AND NOT LFQUICK_BUILD_UI)
    message(FATAL_ERROR "LFQUICK_BUILD_DATAVIEW requires LFQUICK_BUILD_UI=ON")
endif()

if(LFQUICK_BUILD_MEDICAL_DEMO AND (NOT LFQUICK_BUILD_UI OR NOT LFQUICK_BUILD_DATAVIEW OR NOT LFQUICK_BUILD_PLATFORM))
    message(FATAL_ERROR "LFQUICK_BUILD_MEDICAL_DEMO requires UI, DataView and Platform modules")
endif()

if(LFQUICK_BUILD_UI)
    add_subdirectory(src/ui)
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
