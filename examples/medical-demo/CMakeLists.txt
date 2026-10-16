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

set(lfquick_medical_demo_qml_files
    qml/App.qml
    qml/DashboardPage.qml
    qml/SamplesPage.qml
    qml/SampleRegisterPage.qml
    qml/SampleRackPage.qml
    qml/ResultsPage.qml
    qml/ResultsReviewPage.qml
    qml/InstrumentPage.qml
    qml/AlarmsPage.qml
    qml/DeveloperPage.qml
)

if(LFQUICK_BUILD_QT_GRAPHS_ADAPTER)
    target_link_libraries(LfQuickMedicalDemo PRIVATE LfQuick::Charts)
    list(APPEND lfquick_medical_demo_qml_files qml/ChartsPage.qml)
endif()

qt_add_qml_module(LfQuickMedicalDemo
    URI LfQuick.MedicalDemo
    VERSION 1.0
    QML_FILES
        ${lfquick_medical_demo_qml_files}
)

install(TARGETS LfQuickMedicalDemo
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

qt_generate_deploy_qml_app_script(
    TARGET LfQuickMedicalDemo
    OUTPUT_SCRIPT lfquick_medical_demo_deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${lfquick_medical_demo_deploy_script})
