qt_add_library(LfQuickUi
    src/QuickPageProvider.cpp
    src/NavigationGuardState.cpp
    src/NavigationGuardController.cpp
    src/QuickRuntime.cpp
)
add_library(LfQuick::Ui ALIAS LfQuickUi)

target_compile_definitions(LfQuickUi PRIVATE LFQUICKUI_LIBRARY)

target_include_directories(LfQuickUi
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include/LfQuick/Ui
)

set_source_files_properties(qml/Theme.qml PROPERTIES QT_QML_SINGLETON_TYPE TRUE)
set_source_files_properties(qml/Tokens.qml PROPERTIES QT_QML_SINGLETON_TYPE TRUE)

qt_add_qml_module(LfQuickUi
    URI LfQuick
    VERSION 1.0
    RESOURCE_PREFIX /qt/qml
    SOURCES
        include/LfQuick/Ui/QmlErrorCollector.h
        src/QmlErrorCollector.cpp
        include/LfQuick/Ui/OverlayCoordinator.h
        src/OverlayCoordinator.cpp
        include/LfQuick/Ui/FocusNavigator.h
        src/FocusNavigator.cpp
        include/LfQuick/Ui/PermissionEvaluator.h
        src/PermissionEvaluator.cpp
        include/LfQuick/Ui/LfAttached.h
        src/LfAttached.cpp
        include/LfQuick/Ui/ValidationEvaluator.h
        src/ValidationEvaluator.cpp
        include/LfQuick/Ui/ValidationRuleTable.h
        src/ValidationRuleTable.cpp
        include/LfQuick/Ui/ValidationFormState.h
        src/ValidationFormState.cpp
    QML_FILES
        qml/LfPage.qml
        qml/QuickHostRoot.qml
        qml/PageLoadError.qml
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
        include/LfQuick/Ui/QmlErrorCollector.h
        include/LfQuick/Ui/OverlayCoordinator.h
        include/LfQuick/Ui/FocusNavigator.h
        include/LfQuick/Ui/NavigationGuardState.h
        include/LfQuick/Ui/NavigationGuardController.h
        include/LfQuick/Ui/PermissionEvaluator.h
        include/LfQuick/Ui/LfAttached.h
        include/LfQuick/Ui/ValidationEvaluator.h
        include/LfQuick/Ui/ValidationRuleTable.h
        include/LfQuick/Ui/ValidationFormState.h
        include/LfQuick/Ui/QuickRuntime.h
)

target_link_libraries(LfQuickUi
    PUBLIC
        LfQuick::Core
        LfQuick::Runtime
        Qt6::Gui
        Qt6::Qml
        Qt6::QmlIntegration
        Qt6::Quick
        Qt6::QuickControls2
        Qt6::QuickLayouts
)
