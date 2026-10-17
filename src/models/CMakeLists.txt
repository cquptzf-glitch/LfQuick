qt_add_library(LfQuickModels)
add_library(LfQuick::Models ALIAS LfQuickModels)

target_compile_definitions(LfQuickModels PRIVATE LFQUICKMODELS_LIBRARY)

target_include_directories(LfQuickModels
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include/LfQuick/Models
)

qt_add_qml_module(LfQuickModels
    URI LfQuick.Models
    VERSION 1.0
    RESOURCE_PREFIX /qt/qml
    OUTPUT_TARGETS _lfquick_models_qml_output_targets
    SOURCES
        include/LfQuick/Models/ObjectListModel.h
        src/ObjectListModel.cpp
        include/LfQuick/Models/ScriptModel.h
        src/ScriptModel.cpp
        include/LfQuick/Models/ObjectRepeater.h
        src/ObjectRepeater.cpp
)
lfquick_register_qml_output_targets(
    LfQuickModels ${_lfquick_models_qml_output_targets})

target_sources(LfQuickModels PUBLIC
    FILE_SET HEADERS
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include
    FILES
        include/LfQuick/Models/LfQuickModelsGlobal.h
        include/LfQuick/Models/ObjectListModel.h
        include/LfQuick/Models/ScriptModel.h
        include/LfQuick/Models/ObjectRepeater.h
)

target_link_libraries(LfQuickModels
    PUBLIC
        Qt6::Core
        Qt6::Qml
        Qt6::QmlIntegration
)
