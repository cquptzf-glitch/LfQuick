add_library(LfQuickPlatform INTERFACE)
add_library(LfQuick::Platform ALIAS LfQuickPlatform)
target_link_libraries(LfQuickPlatform INTERFACE LfQuick::Core Qt6::Core)
