
add_library(LfQuickRuntime
    src/PageRegistry.cpp
    src/PageProviderRegistry.cpp
    src/PageLifecycle.cpp
    src/PageRouter.cpp
)
add_library(LfQuick::Runtime ALIAS LfQuickRuntime)

target_compile_definitions(LfQuickRuntime PRIVATE LFQUICKRUNTIME_LIBRARY)

target_include_directories(LfQuickRuntime
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_link_libraries(LfQuickRuntime PUBLIC LfQuick::Core Qt6::Core)

target_sources(LfQuickRuntime PUBLIC
    FILE_SET HEADERS
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include
    FILES
        include/LfQuick/Runtime/LfQuickRuntimeGlobal.h
        include/LfQuick/Runtime/PageDescriptor.h
        include/LfQuick/Runtime/IPageInstance.h
        include/LfQuick/Runtime/IPageProvider.h
        include/LfQuick/Runtime/PageRegistry.h
        include/LfQuick/Runtime/PageProviderRegistry.h
        include/LfQuick/Runtime/PageLifecycle.h
        include/LfQuick/Runtime/PageRouter.h
)
