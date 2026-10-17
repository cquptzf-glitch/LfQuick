add_library(LfQuickCore
    src/Error.cpp
    src/OperationHandle.cpp
    src/Event.cpp
    src/Validation.cpp
    src/Permission.cpp
    src/Route.cpp
    src/Telemetry.cpp
)
add_library(LfQuick::Core ALIAS LfQuickCore)

target_compile_definitions(LfQuickCore PRIVATE LFQUICKCORE_LIBRARY)

target_include_directories(LfQuickCore
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_link_libraries(LfQuickCore PUBLIC Qt6::Core)

target_sources(LfQuickCore PUBLIC
    FILE_SET HEADERS
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include
    FILES
        include/LfQuick/Core/LfQuickCoreGlobal.h
        include/LfQuick/Core/Error.h
        include/LfQuick/Core/Result.h
        include/LfQuick/Core/OperationHandle.h
        include/LfQuick/Core/Validation.h
        include/LfQuick/Core/Permission.h
        include/LfQuick/Core/Command.h
        include/LfQuick/Core/Event.h
        include/LfQuick/Core/Route.h
        include/LfQuick/Core/Telemetry.h
)
