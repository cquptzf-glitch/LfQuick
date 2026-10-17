qt_add_library(LfQuickBrandTestStyle)
add_library(LfQuick::BrandTestStyle ALIAS LfQuickBrandTestStyle)

set_source_files_properties(qml/BrandTokens.qml PROPERTIES QT_QML_SINGLETON_TYPE TRUE)

qt_add_qml_module(LfQuickBrandTestStyle
    URI LfQuick.BrandTest
    VERSION 1.0
    RESOURCE_PREFIX /qt/qml
    OUTPUT_TARGETS _lfquick_brand_qml_output_targets
    QML_FILES
        qml/BrandTokens.qml
        qml/BrandButton.qml
        qml/BrandTextField.qml
        qml/BrandSwitch.qml
        qml/BrandDialog.qml
)
lfquick_register_qml_output_targets(
    LfQuickBrandTestStyle ${_lfquick_brand_qml_output_targets})

target_link_libraries(LfQuickBrandTestStyle
    PUBLIC
        Qt6::Gui
        Qt6::Qml
        Qt6::QmlIntegration
        Qt6::Quick
        Qt6::QuickControls2
        Qt6::QuickLayouts
)
