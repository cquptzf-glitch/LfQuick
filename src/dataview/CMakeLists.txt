qt_add_library(LfQuickDataView)
add_library(LfQuick::DataView ALIAS LfQuickDataView)

target_compile_definitions(LfQuickDataView PRIVATE LFQUICKDATAVIEW_LIBRARY)

target_include_directories(LfQuickDataView
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include/LfQuick/DataView
)

qt_add_qml_module(LfQuickDataView
    URI LfQuick.DataView
    VERSION 1.0
    RESOURCE_PREFIX /qt/qml
    SOURCES
        include/LfQuick/DataView/DataTableModel.h
        src/DataTableModel.cpp
        include/LfQuick/DataView/DataFilterProxyModel.h
        src/DataFilterProxyModel.cpp
        include/LfQuick/DataView/ModelRoleAdapter.h
        src/ModelRoleAdapter.cpp
        include/LfQuick/DataView/ColumnStateModel.h
        src/ColumnStateModel.cpp
        include/LfQuick/DataView/ColumnConfigStore.h
        src/ColumnConfigStore.cpp
        include/LfQuick/DataView/DataEditTransaction.h
        src/DataEditTransaction.cpp
        include/LfQuick/DataView/DataGridSelectionController.h
        src/DataGridSelectionController.cpp
    QML_FILES
        qml/DataGrid.qml
        qml/DataGridTelemetry.qml
        qml/DataGridKeyboardController.qml
        qml/DataGridEditorDelegate.qml
        qml/DataGridCheckDelegate.qml
        qml/DataGridCheckEditorDelegate.qml
        qml/DataGridComboDelegate.qml
        qml/DataGridComboEditorDelegate.qml
        qml/DataGridProgressDelegate.qml
        qml/DataGridIconDelegate.qml
        qml/DataGridBadgeDelegate.qml
        qml/DataGridButtonDelegate.qml
        qml/DataGridCustomDelegate.qml
)

target_sources(LfQuickDataView PUBLIC
    FILE_SET HEADERS
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include
    FILES
        include/LfQuick/DataView/LfQuickDataViewGlobal.h
        include/LfQuick/DataView/DataTableModel.h
        include/LfQuick/DataView/DataFilterProxyModel.h
        include/LfQuick/DataView/ModelRoleAdapter.h
        include/LfQuick/DataView/ColumnStateModel.h
        include/LfQuick/DataView/ColumnConfigStore.h
        include/LfQuick/DataView/DataEditTransaction.h
        include/LfQuick/DataView/DataGridSelectionController.h
)

target_link_libraries(LfQuickDataView
    PUBLIC
        LfQuick::Ui
        Qt6::Core
        Qt6::Qml
        Qt6::QmlIntegration
        Qt6::Quick
        Qt6::QuickControls2
)
