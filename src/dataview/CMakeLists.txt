qt_add_library(LfQuickDataView
    src/DataTableModel.cpp
    src/DataFilterProxyModel.cpp
)
add_library(LfQuick::DataView ALIAS LfQuickDataView)

target_compile_definitions(LfQuickDataView PRIVATE LFQUICKDATAVIEW_LIBRARY)

target_include_directories(LfQuickDataView
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

qt_add_qml_module(LfQuickDataView
    URI LfQuick.DataView
    VERSION 1.0
    RESOURCE_PREFIX /qt/qml
    QML_FILES
        qml/DataGrid.qml
)

target_sources(LfQuickDataView PUBLIC
    FILE_SET HEADERS
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include
    FILES
        include/LfQuick/DataView/LfQuickDataViewGlobal.h
        include/LfQuick/DataView/DataTableModel.h
        include/LfQuick/DataView/DataFilterProxyModel.h
)

target_link_libraries(LfQuickDataView
    PUBLIC
        LfQuick::Ui
        Qt6::Core
        Qt6::Qml
        Qt6::Quick
        Qt6::QuickControls2
)
