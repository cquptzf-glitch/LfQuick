qt_add_executable(LfQuickHybridDemo
    cpp/main.cpp
)

target_link_libraries(LfQuickHybridDemo
    PRIVATE
        LfQuick::Core
        LfQuick::Runtime
        LfQuick::Ui
        LfQuick::WidgetBridge
        Qt6::Widgets
        Qt6::Qml
        Qt6::Quick
        Qt6::QuickControls2
)

if(LFQUICK_BUILD_DIAGNOSTICS)
    target_link_libraries(LfQuickHybridDemo PRIVATE LfQuick::Diagnostics)
    target_compile_definitions(LfQuickHybridDemo PRIVATE LFQUICK_HAS_DIAGNOSTICS=1)
endif()

qt_add_qml_module(LfQuickHybridDemo
    URI LfQuick.HybridDemo
    VERSION 1.0
    RESOURCE_PREFIX /qt/qml
    QML_FILES
        qml/HybridQuickPage.qml
)

install(TARGETS LfQuickHybridDemo
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

qt_generate_deploy_qml_app_script(
    TARGET LfQuickHybridDemo
    OUTPUT_SCRIPT lfquick_hybrid_demo_deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${lfquick_hybrid_demo_deploy_script})
