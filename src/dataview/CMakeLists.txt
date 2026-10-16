
add_library(LfQuickDataView INTERFACE)
add_library(LfQuick::DataView ALIAS LfQuickDataView)

target_link_libraries(LfQuickDataView INTERFACE LfQuick::Ui Qt6::Core Qt6::Quick)
