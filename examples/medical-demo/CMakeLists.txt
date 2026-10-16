qt_add_executable(LfQuickMedicalDemo
    cpp/main.cpp
)

target_link_libraries(LfQuickMedicalDemo
    PRIVATE
        LfQuick::Core
        LfQuick::Runtime
        LfQuick::Ui
        LfQuick::DataView
        LfQuick::Platform
        Qt6::Gui
        Qt6::Qml
        Qt6::Quick
        Qt6::QuickControls2
        Qt6::QuickLayouts
)

qt_add_qml_module(LfQuickMedicalDemo
    URI LfQuick.MedicalDemo
    VERSION 1.0
    QML_FILES
        qml/App.qml
        qml/NavigationRail.qml
        qml/DashboardPage.qml
        qml/SamplesPage.qml
        qml/ResultsPage.qml
        qml/InstrumentPage.qml
        qml/AlarmsPage.qml
        qml/DeveloperPage.qml
)
