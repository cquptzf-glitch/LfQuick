add_library(LfQuickCharts INTERFACE)
add_library(LfQuick::Charts ALIAS LfQuickCharts)
target_link_libraries(LfQuickCharts INTERFACE LfQuick::Ui Qt6::Core Qt6::Quick)
