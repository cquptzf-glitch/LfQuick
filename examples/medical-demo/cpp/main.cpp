#include <LfQuick/DataView/DataTableModel.h>
#include <LfQuick/Platform/InputRouter.h>
#include <LfQuick/Platform/KeyboardWedgeScanner.h>
#include <LfQuick/Ui/OverlayCoordinator.h>

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickStyle>
#include <QTimer>
#include <QVariantList>

using namespace LfQuick;

namespace {

void seedSampleModel(DataTableModel &model, int count)
{
    model.setColumnKeys({
        QStringLiteral("sampleId"),
        QStringLiteral("barcode"),
        QStringLiteral("sampleType"),
        QStringLiteral("rack"),
        QStringLiteral("position"),
        QStringLiteral("priority"),
        QStringLiteral("status"),
        QStringLiteral("testCount"),
        QStringLiteral("result"),
        QStringLiteral("createTime")
    });

    model.setColumnTitles({
        QStringLiteral("Sample ID"),
        QStringLiteral("Barcode"),
        QStringLiteral("Type"),
        QStringLiteral("Rack"),
        QStringLiteral("Position"),
        QStringLiteral("Priority"),
        QStringLiteral("Status"),
        QStringLiteral("Tests"),
        QStringLiteral("Result"),
        QStringLiteral("Created")
    });

    QVariantList rows;
    rows.reserve(count);

    const QStringList statuses{
        QStringLiteral("Pending"),
        QStringLiteral("Running"),
        QStringLiteral("Completed"),
        QStringLiteral("Retest"),
        QStringLiteral("Failed")
    };

    for (int i = 0; i < count; ++i) {
        const QString id = QStringLiteral("S-%1").arg(i + 1, 6, 10, QLatin1Char('0'));
        rows.append(QVariantMap{
            {QStringLiteral("sampleId"), id},
            {QStringLiteral("barcode"), QStringLiteral("BC%1").arg(i + 100000)},
            {QStringLiteral("sampleType"), i % 3 == 0 ? QStringLiteral("Serum") : QStringLiteral("Plasma")},
            {QStringLiteral("rack"), QStringLiteral("R%1").arg(i / 100 + 1)},
            {QStringLiteral("position"), (i % 100) + 1},
            {QStringLiteral("priority"), i % 17 == 0 ? QStringLiteral("STAT") : QStringLiteral("Routine")},
            {QStringLiteral("status"), statuses.at(i % statuses.size())},
            {QStringLiteral("testCount"), (i % 8) + 1},
            {QStringLiteral("result"), QString::number(0.5 + (i % 250) / 10.0, 'f', 2)},
            {QStringLiteral("createTime"), QStringLiteral("10:%1:%2")
                .arg((i / 60) % 60, 2, 10, QLatin1Char('0'))
                .arg(i % 60, 2, 10, QLatin1Char('0'))}
        });
    }

    model.setRows(rows);
}

void seedResultModel(DataTableModel &model, int count)
{
    model.setColumnKeys({
        QStringLiteral("sampleId"),
        QStringLiteral("assay"),
        QStringLiteral("result"),
        QStringLiteral("unit"),
        QStringLiteral("flag"),
        QStringLiteral("status"),
        QStringLiteral("time")
    });

    model.setColumnTitles({
        QStringLiteral("Sample ID"),
        QStringLiteral("Assay"),
        QStringLiteral("Result"),
        QStringLiteral("Unit"),
        QStringLiteral("Flag"),
        QStringLiteral("Status"),
        QStringLiteral("Time")
    });

    QVariantList rows;
    rows.reserve(count);

    const QStringList assays{
        QStringLiteral("GLU"),
        QStringLiteral("ALT"),
        QStringLiteral("AST"),
        QStringLiteral("CRP"),
        QStringLiteral("TSH")
    };

    for (int i = 0; i < count; ++i) {
        const double result = 2.0 + (i % 1000) / 10.0;
        rows.append(QVariantMap{
            {QStringLiteral("sampleId"), QStringLiteral("S-%1").arg(i + 1, 6, 10, QLatin1Char('0'))},
            {QStringLiteral("assay"), assays.at(i % assays.size())},
            {QStringLiteral("result"), QString::number(result, 'f', 2)},
            {QStringLiteral("unit"), i % 2 ? QStringLiteral("U/L") : QStringLiteral("mmol/L")},
            {QStringLiteral("flag"), i % 29 == 0 ? QStringLiteral("High") : QStringLiteral("Normal")},
            {QStringLiteral("status"), i % 11 == 0 ? QStringLiteral("Pending Review") : QStringLiteral("Reviewed")},
            {QStringLiteral("time"), QStringLiteral("10:%1:%2")
                .arg((i / 60) % 60, 2, 10, QLatin1Char('0'))
                .arg(i % 60, 2, 10, QLatin1Char('0'))}
        });
    }

    model.setRows(rows);
}

} // namespace

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    QQuickStyle::setStyle(QStringLiteral("Basic"));

    DataTableModel sampleModel;
    DataTableModel resultModel;
    seedSampleModel(sampleModel, 10000);
    seedResultModel(resultModel, 5000);

    KeyboardWedgeScanner scanner;
    scanner.session()->setMinimumLength(4);

    InputRouter inputRouter;
    inputRouter.setScanner(&scanner);
    inputRouter.installOn(&app);

    OverlayCoordinator overlayCoordinator;

    QTimer liveDataTimer(&app);
    QObject::connect(&liveDataTimer, &QTimer::timeout, &sampleModel, [&sampleModel] {
        static int cursor = 0;
        const int rowCount = sampleModel.rowCount();
        if (rowCount == 0)
            return;

        for (int i = 0; i < 3; ++i) {
            const int row = (cursor + i) % rowCount;
            sampleModel.setCell(row, QStringLiteral("status"),
                                row % 9 == 0 ? QStringLiteral("Completed")
                                             : QStringLiteral("Running"));
        }
        cursor = (cursor + 3) % rowCount;
    });
    liveDataTimer.start(800);

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty(QStringLiteral("sampleModel"), &sampleModel);
    engine.rootContext()->setContextProperty(QStringLiteral("resultModel"), &resultModel);
    engine.rootContext()->setContextProperty(QStringLiteral("scanner"), &scanner);
    engine.rootContext()->setContextProperty(QStringLiteral("inputRouter"), &inputRouter);
    engine.rootContext()->setContextProperty(QStringLiteral("overlayCoordinator"), &overlayCoordinator);

    QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed,
                     &app, [] { QCoreApplication::exit(-1); },
                     Qt::QueuedConnection);
    engine.loadFromModule(QStringLiteral("LfQuick.MedicalDemo"),
                          QStringLiteral("App"));

    return app.exec();
}
