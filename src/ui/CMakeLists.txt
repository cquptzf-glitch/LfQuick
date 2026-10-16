
qt_add_library(LfQuickUi
    src/QuickPageProvider.cpp
)
add_library(LfQuick::Ui ALIAS LfQuickUi)

target_include_directories(LfQuickUi
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

set_source_files_properties(qml/Theme.qml PROPERTIES QT_QML_SINGLETON_TYPE TRUE)
set_source_files_properties(qml/Tokens.qml PROPERTIES QT_QML_SINGLETON_TYPE TRUE)

qt_add_qml_module(LfQuickUi
    URI LfQuick
    VERSION 1.0
    RESOURCE_PREFIX /qt/qml
    QML_FILES
        qml/LfPage.qml
        qml/QuickHostRoot.qml
        qml/Theme.qml
        qml/Tokens.qml
        qml/OverlayHost.qml
        qml/BusyOverlay.qml
)

target_sources(LfQuickUi PUBLIC
    FILE_SET HEADERS
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include
    FILES
        include/LfQuick/Ui/QuickPageProvider.h
)

target_link_libraries(LfQuickUi
    PUBLIC
        LfQuick::Core
        LfQuick::Runtime
        Qt6::Gui
        Qt6::Qml
        Qt6::Quick
        Qt6::QuickControls2
        Qt6::QuickLayouts
)
