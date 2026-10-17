qt_add_library(LfQuickFluentStyle)
add_library(LfQuick::FluentStyle ALIAS LfQuickFluentStyle)

set_source_files_properties(qml/FluentTokens.qml PROPERTIES QT_QML_SINGLETON_TYPE TRUE)

qt_add_qml_module(LfQuickFluentStyle
    URI LfQuick.Fluent
    VERSION 1.0
    RESOURCE_PREFIX /qt/qml
    QML_FILES
        qml/FluentTokens.qml
        qml/FluentIcon.qml
        qml/FluentButton.qml
        qml/FluentTextField.qml
        qml/FluentCheckBox.qml
        qml/FluentRadioButton.qml
        qml/FluentSwitch.qml
        qml/FluentComboBox.qml
        qml/FluentSlider.qml
        qml/FluentProgressBar.qml
)

target_link_libraries(LfQuickFluentStyle
    PUBLIC
        Qt6::Gui
        Qt6::Qml
        Qt6::QmlIntegration
        Qt6::Quick
        Qt6::QuickControls2
        Qt6::QuickLayouts
)
