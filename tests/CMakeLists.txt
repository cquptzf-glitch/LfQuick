
find_package(Qt6 6.8.3 REQUIRED COMPONENTS Test)

qt_add_executable(tst_core
    unit/tst_core.cpp
)
target_link_libraries(tst_core PRIVATE LfQuick::Core Qt6::Test)
add_test(NAME tst_core COMMAND tst_core)

qt_add_executable(tst_runtime
    unit/tst_runtime.cpp
)
target_link_libraries(tst_runtime PRIVATE LfQuick::Runtime Qt6::Test)
add_test(NAME tst_runtime COMMAND tst_runtime)
