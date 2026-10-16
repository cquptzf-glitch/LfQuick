add_library(LfQuickDiagnostics INTERFACE)
add_library(LfQuick::Diagnostics ALIAS LfQuickDiagnostics)
target_link_libraries(LfQuickDiagnostics INTERFACE LfQuick::Runtime Qt6::Core)
