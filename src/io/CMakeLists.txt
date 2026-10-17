find_package(Qt6 6.8.3 REQUIRED COMPONENTS Core Qml QmlIntegration Network)

qt_add_library(LfQuickIo
    src/IpcFrameCodec.cpp
)
add_library(LfQuick::Io ALIAS LfQuickIo)

target_compile_definitions(LfQuickIo PRIVATE LFQUICKIO_LIBRARY)

target_include_directories(LfQuickIo
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include/LfQuick/Io
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

qt_add_qml_module(LfQuickIo
    URI LfQuick.Io
    VERSION 1.0
    RESOURCE_PREFIX /qt/qml
    OUTPUT_TARGETS _lfquick_io_qml_output_targets
    SOURCES
        include/LfQuick/Io/FileWatcher.h
        src/FileWatcher.cpp
        include/LfQuick/Io/FileDocument.h
        src/FileDocument.cpp
        include/LfQuick/Io/JsonDocument.h
        src/JsonDocument.cpp
        include/LfQuick/Io/DataStream.h
        src/DataStream.cpp
        include/LfQuick/Io/ProcessRunner.h
        src/ProcessRunner.cpp
        include/LfQuick/Io/LocalIpcServer.h
        src/LocalIpcServer.cpp
        include/LfQuick/Io/LocalIpcClient.h
        src/LocalIpcClient.cpp
)
lfquick_register_qml_output_targets(LfQuickIo ${_lfquick_io_qml_output_targets})

target_sources(LfQuickIo PUBLIC
    FILE_SET HEADERS
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include
    FILES
        include/LfQuick/Io/LfQuickIoGlobal.h
        include/LfQuick/Io/FileWatcher.h
        include/LfQuick/Io/FileDocument.h
        include/LfQuick/Io/JsonDocument.h
        include/LfQuick/Io/DataStream.h
        include/LfQuick/Io/ProcessRunner.h
        include/LfQuick/Io/LocalIpcServer.h
        include/LfQuick/Io/LocalIpcClient.h
)

target_link_libraries(LfQuickIo
    PUBLIC
        Qt6::Core
        Qt6::Qml
        Qt6::QmlIntegration
        Qt6::Network
)
