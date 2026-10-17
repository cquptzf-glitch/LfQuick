#include <QDebug>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QQmlError>
#include <QTest>

class UiControlsSmokeTest : public QObject
{
    Q_OBJECT

private slots:
    void allBaseControlsInstantiate()
    {
        QQmlEngine engine;
        QQmlComponent component(&engine);
        component.setData(R"QML(
import QtQuick
import LfQuick

Item {
    width: 1000
    height: 800

    LfButton { text: "Button" }
    LfTextField { text: "Text" }
    LfCheckBox { text: "Check"; checked: true }
    LfRadioButton { text: "Radio"; checked: true }
    LfSwitch { text: "Switch"; checked: true }
    LfComboBox { model: ["A", "B"] }
    LfSlider { from: 0; to: 100; value: 25 }
    LfProgressBar { from: 0; to: 100; value: 25 }
    LfSearchBox { text: "sample" }
    LfFilterInput { text: "ready" }
    LfPasswordField { text: "secret" }
    LfStatusIndicator { status: LfStatusIndicator.Success; text: "Ready" }
    LfNumberInput { from: 0; to: 100; value: 42 }
    LfRangeInput { from: 0; to: 100; first.value: 20; second.value: 80 }
    LfProgressRing { running: true }
    LfDatePicker { value: new Date(2026, 9, 17) }
    LfTimePicker { value: new Date(2026, 9, 17, 10, 30) }
    LfDateTimePicker { value: new Date(2026, 9, 17, 10, 30) }
    LfDateRangePicker {
        startDate: new Date(2026, 9, 1)
        endDate: new Date(2026, 9, 17)
    }
}
)QML", QUrl(QStringLiteral("qrc:/tests/LfQuickBaseControls.qml")));

        if (component.isError()) {
            for (const QQmlError &error : component.errors())
                qWarning().noquote() << error.toString();
        }

        QObject *object = component.create();
        if (!object) {
            for (const QQmlError &error : component.errors())
                qWarning().noquote() << error.toString();
        }
        QVERIFY2(object, qPrintable(component.errorString()));
        delete object;
    }
};

QTEST_MAIN(UiControlsSmokeTest)
#include "tst_ui_controls.moc"
