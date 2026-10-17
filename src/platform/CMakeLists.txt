qt_add_library(LfQuickPlatform
    src/ScannerSession.cpp
    src/KeyboardWedgeScanner.cpp
    src/MockScanner.cpp
    src/InputTarget.cpp
    src/InputRouter.cpp
    src/HotkeyRouter.cpp
    src/SystemAppearance.cpp
    src/ScreenService.cpp
    src/ClipboardService.cpp
    src/FileWatcher.cpp
    src/RuntimeCapabilities.cpp
)
add_library(LfQuick::Platform ALIAS LfQuickPlatform)

target_compile_definitions(LfQuickPlatform
    PRIVATE
        LFQUICKPLATFORM_LIBRARY
        $<$<BOOL:${LFQUICK_BUILD_WIDGET_BRIDGE}>:LFQUICK_CAP_WIDGET_BRIDGE=1>
        $<$<BOOL:${LFQUICK_BUILD_CHARTS}>:LFQUICK_CAP_CHARTS=1>
        $<$<BOOL:${LFQUICK_BUILD_QT_GRAPHS_ADAPTER}>:LFQUICK_CAP_QT_GRAPHS=1>
        $<$<BOOL:${LFQUICK_BUILD_QRANGE_MODEL_ADAPTER}>:LFQUICK_CAP_QRANGE_MODEL=1>
        $<$<BOOL:${LFQUICK_EXPERIMENTAL_CANVAS_PAINTER}>:LFQUICK_CAP_CANVAS_PAINTER=1>
        $<$<BOOL:${LFQUICK_BUILD_DIAGNOSTICS}>:LFQUICK_CAP_DIAGNOSTICS=1>
        $<$<BOOL:${LFQUICK_BUILD_SCREENSHOT_TOOL}>:LFQUICK_CAP_SCREENSHOT_TOOL=1>
        $<$<BOOL:${LFQUICK_ENABLE_HOT_RELOAD}>:LFQUICK_CAP_HOT_RELOAD=1>
        $<$<BOOL:${LFQUICK_EXPERIMENTAL_STYLEKIT}>:LFQUICK_CAP_STYLEKIT=1>
)

target_include_directories(LfQuickPlatform
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_sources(LfQuickPlatform PUBLIC
    FILE_SET HEADERS
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include
    FILES
        include/LfQuick/Platform/LfQuickPlatformGlobal.h
        include/LfQuick/Platform/IInputSource.h
        include/LfQuick/Platform/ScannerSession.h
        include/LfQuick/Platform/KeyboardWedgeScanner.h
        include/LfQuick/Platform/MockScanner.h
        include/LfQuick/Platform/InputTarget.h
        include/LfQuick/Platform/InputRouter.h
        include/LfQuick/Platform/HotkeyRouter.h
        include/LfQuick/Platform/SystemAppearance.h
        include/LfQuick/Platform/ScreenService.h
        include/LfQuick/Platform/ClipboardService.h
        include/LfQuick/Platform/FileWatcher.h
        include/LfQuick/Platform/RuntimeCapabilities.h
)

target_link_libraries(LfQuickPlatform
    PUBLIC
        LfQuick::Core
        Qt6::Core
        Qt6::Gui
)
