qt_add_library(LfQuickPlatform
    src/ScannerSession.cpp
    src/KeyboardWedgeScanner.cpp
    src/InputRouter.cpp
)
add_library(LfQuick::Platform ALIAS LfQuickPlatform)

target_compile_definitions(LfQuickPlatform PRIVATE LFQUICKPLATFORM_LIBRARY)

target_include_directories(LfQuickPlatform
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_INSTALL_INCLUDEDIR}
)

target_sources(LfQuickPlatform PUBLIC
    FILE_SET HEADERS
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include
    FILES
        include/LfQuick/Platform/LfQuickPlatformGlobal.h
        include/LfQuick/Platform/ScannerSession.h
        include/LfQuick/Platform/KeyboardWedgeScanner.h
        include/LfQuick/Platform/InputRouter.h
)

target_link_libraries(LfQuickPlatform
    PUBLIC
        LfQuick::Core
        Qt6::Core
        Qt6::Gui
)

set_target_properties(LfQuickPlatform PROPERTIES
    AUTOMOC ON
    AUTOUIC OFF
    AUTORCC OFF
)
