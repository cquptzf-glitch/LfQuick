qt_add_library(LfQuickUi
    src/QuickPageProvider.cpp
    src/OverlayCoordinator.cpp
    src/FocusNavigator.cpp
    src/QuickRuntime.cpp
)
add_library(LfQuick::Ui ALIAS LfQuickUi)

target_compile_definitions(LfQuickUi PRIVATE LFQUICKUI_LIBRARY)

target_include_directories(LfQuickUi
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_INSTALL_INCLUDEDIR}
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
        qml/FocusDebugPanel.qml
)

target_sources(LfQuickUi PUBLIC
    FILE_SET HEADERS
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include
    FILES
        include/LfQuick/Ui/LfQuickUiGlobal.h
        include/LfQuick/Ui/QuickPageProvider.h
        include/LfQuick/Ui/OverlayCoordinator.h
        include/LfQuick/Ui/FocusNavigator.h
        include/LfQuick/Ui/QuickRuntime.h
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
