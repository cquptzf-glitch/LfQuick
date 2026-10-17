add_library(LfQuickRuntime
    src/IPageProvider.cpp
    src/IWindowProvider.cpp
    src/PageRegistry.cpp
    src/PageProviderRegistry.cpp
    src/PageLifecycle.cpp
    src/PageRouter.cpp
    src/TelemetryPageProvider.cpp
    src/RuntimeTelemetryObserver.cpp
    src/WindowProviderRegistry.cpp
    src/WindowRouter.cpp
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
        include/LfQuick/Runtime/IStatefulInstance.h
        include/LfQuick/Runtime/PageDescriptor.h
        include/LfQuick/Runtime/IPageInstance.h
        include/LfQuick/Runtime/IPageProvider.h
        include/LfQuick/Runtime/INavigationGuard.h
        include/LfQuick/Runtime/PageRegistry.h
        include/LfQuick/Runtime/PageProviderRegistry.h
        include/LfQuick/Runtime/PageLifecycle.h
        include/LfQuick/Runtime/PageRouter.h
        include/LfQuick/Runtime/TelemetryPageProvider.h
        include/LfQuick/Runtime/RuntimeTelemetryObserver.h
        include/LfQuick/Runtime/WindowDescriptor.h
        include/LfQuick/Runtime/IWindowInstance.h
        include/LfQuick/Runtime/IWindowProvider.h
        include/LfQuick/Runtime/WindowProviderRegistry.h
        include/LfQuick/Runtime/WindowRouter.h
)
