#include <LfQuick/Core/Permission.h>
#include <LfQuick/DataView/DataTableModel.h>
#include <LfQuick/Platform/InputRouter.h>
#include <LfQuick/Platform/KeyboardWedgeScanner.h>
#include <LfQuick/Ui/OverlayCoordinator.h>
#include <LfQuick/Ui/PermissionEvaluator.h>
#include <LfQuick/Ui/QuickRuntime.h>

#ifdef LFQUICK_HAS_QT_GRAPHS_ADAPTER
#include <LfQuick/Charts/ChartDataModel.h>
#endif

#ifdef LFQUICK_HAS_DIAGNOSTICS
#include <LfQuick/Diagnostics/DiagnosticsModel.h>
#endif

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include <QTimer>
#include <QVariantList>

#include <cmath>
#include <memory>

using namespace LfQuick;

namespace {

class DemoPermissionProvider final : public IPermissionProvider
{
public:
    PermissionState evaluate(QStringView permissionId,
                             const QVariantMap &context) const override
    {
        if (permissionId != QStringView(u"demo.results.review"))
            return PermissionState::Denied;

        const QString role = context.value(QStringLiteral("role"),
                                           QStringLiteral("operator"))
                                 .toString().trimmed().toLower();
        if (role == QStringLiteral("reviewer"))
            return PermissionState::Allow;
        if (role == QStringLiteral("operator"))
            return PermissionState::ReadOnly;
        if (role == QStringLiteral("guest"))
            return PermissionState::Denied;
        return PermissionState::Disabled;
    }
};

void seedSampleModel(DataTableModel &model, int count)
{
    model.setColumnKeys({
        QStringLiteral("sampleId"), QStringLiteral("barcode"), QStringLiteral("sampleType"),
        QStringLiteral("rack"), QStringLiteral("position"), QStringLiteral("priority"),
        QStringLiteral("status"), QStringLiteral("testCount"), QStringLiteral("result"),
        QStringLiteral("createTime")
    });
    model.setColumnTitles({
        QStringLiteral("Sample ID"), QStringLiteral("Barcode"), QStringLiteral("Type"),
        QStringLiteral("Rack"), QStringLiteral("Position"), QStringLiteral("Priority"),
        QStringLiteral("Status"), QStringLiteral("Tests"), QStringLiteral("Result"),
        QStringLiteral("Created")
    });

    QVariantList rows;
    rows.reserve(count);
    const QStringList statuses{
        QStringLiteral("Pending"), QStringLiteral("Running"),
        QStringLiteral("Completed"), QStringLiteral("Retest"), QStringLiteral("Failed")
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
        QStringLiteral("sampleId"), QStringLiteral("assay"), QStringLiteral("result"),
        QStringLiteral("unit"), QStringLiteral("flag"), QStringLiteral("status"),
        QStringLiteral("time")
    });
    model.setColumnTitles({
        QStringLiteral("Sample ID"), QStringLiteral("Assay"), QStringLiteral("Result"),
        QStringLiteral("Unit"), QStringLiteral("Flag"), QStringLiteral("Status"),
        QStringLiteral("Time")
    });

    QVariantList rows;
    rows.reserve(count);
    const QStringList assays{
        QStringLiteral("GLU"), QStringLiteral("ALT"), QStringLiteral("AST"),
        QStringLiteral("CRP"), QStringLiteral("TSH")
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

#ifdef LFQUICK_HAS_QT_GRAPHS_ADAPTER
void seedSyntheticChartModel(ChartDataModel &model)
{
    QVariantList points;
    points.reserve(120);
    for (int i = 0; i < 120; ++i) {
        const double x = static_cast<double>(i);
        const double y = 50.0 + std::sin(x * 0.12) * 8.0 + std::cos(x * 0.035) * 3.0;
        points.append(QVariantMap{
            {QStringLiteral("x"), x},
            {QStringLiteral("y"), y},
            {QStringLiteral("metadata"), QVariantMap{{QStringLiteral("source"), QStringLiteral("synthetic-demo")}}}
        });
    }
    model.setPoints(points);
}
#endif

} // namespace

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    QQuickStyle::setStyle(QStringLiteral("Basic"));

    DataTableModel sampleModel;
    DataTableModel resultModel;
    seedSampleModel(sampleModel, 10000);
    seedResultModel(resultModel, 5000);

#ifdef LFQUICK_HAS_QT_GRAPHS_ADAPTER
    ChartDataModel syntheticChartModel;
    seedSyntheticChartModel(syntheticChartModel);
#endif

    KeyboardWedgeScanner scanner;
    scanner.session()->setMinimumLength(4);

    InputRouter inputRouter;
    inputRouter.setScanner(&scanner);
    inputRouter.installOn(&app);

    OverlayCoordinator overlayCoordinator;
    PermissionEvaluator permissionEvaluator;
    permissionEvaluator.setProvider(std::make_shared<DemoPermissionProvider>());

    QQmlApplicationEngine engine;
    QuickRuntime quickRuntime(&engine);

#ifdef LFQUICK_HAS_DIAGNOSTICS
    DiagnosticsModel diagnosticsModel;
    diagnosticsModel.setMaxEvents(500);
    diagnosticsModel.setMetric(QStringLiteral("dataview.samples.count"), sampleModel.rowCount());
    diagnosticsModel.setMetric(QStringLiteral("dataview.results.count"), resultModel.rowCount());
    diagnosticsModel.setMetric(QStringLiteral("scanner.enabled"), scanner.isEnabled());
    diagnosticsModel.setMetric(QStringLiteral("runtime.route"), quickRuntime.currentRoute());
    diagnosticsModel.setMetric(QStringLiteral("runtime.depth"), quickRuntime.depth());
    diagnosticsModel.setMetric(QStringLiteral("runtime.singletonCount"), quickRuntime.singletonCount());
    diagnosticsModel.setMetric(QStringLiteral("runtime.retainedLruWeight"), quickRuntime.retainedLruWeight());
    diagnosticsModel.recordEvent(QStringLiteral("app"),
                                 QStringLiteral("startup"),
                                 {{QStringLiteral("syntheticData"), true}},
                                 QStringLiteral("info"),
                                 QStringLiteral("Medical demo diagnostics initialized"));

    QObject::connect(&sampleModel, &DataTableModel::countChanged,
                     &diagnosticsModel, [&] {
        diagnosticsModel.setMetric(QStringLiteral("dataview.samples.count"), sampleModel.rowCount());
    });
    QObject::connect(&resultModel, &DataTableModel::countChanged,
                     &diagnosticsModel, [&] {
        diagnosticsModel.setMetric(QStringLiteral("dataview.results.count"), resultModel.rowCount());
    });
    QObject::connect(&scanner, &KeyboardWedgeScanner::enabledChanged,
                     &diagnosticsModel, [&] {
        diagnosticsModel.setMetric(QStringLiteral("scanner.enabled"), scanner.isEnabled());
    });
    QObject::connect(scanner.session(), &ScannerSession::scanAccepted,
                     &diagnosticsModel, [&](const QString &value) {
        diagnosticsModel.incrementMetric(QStringLiteral("scanner.acceptedCount"));
        diagnosticsModel.recordEvent(QStringLiteral("scanner"),
                                     QStringLiteral("accepted"),
                                     {{QStringLiteral("length"), value.size()}},
                                     QStringLiteral("info"),
                                     QStringLiteral("Scanner input accepted"));
    });
    QObject::connect(scanner.session(), &ScannerSession::scanRejected,
                     &diagnosticsModel, [&](const QString &rawValue, const QString &reason) {
        diagnosticsModel.incrementMetric(QStringLiteral("scanner.rejectedCount"));
        diagnosticsModel.recordEvent(QStringLiteral("scanner"),
                                     QStringLiteral("rejected"),
                                     {{QStringLiteral("length"), rawValue.size()},
                                      {QStringLiteral("reason"), reason}},
                                     QStringLiteral("warning"),
                                     reason);
    });

    QObject::connect(&quickRuntime, &QuickRuntime::currentRouteChanged,
                     &diagnosticsModel, [&] {
        diagnosticsModel.setMetric(QStringLiteral("runtime.route"), quickRuntime.currentRoute());
        diagnosticsModel.setMetric(QStringLiteral("runtime.depth"), quickRuntime.depth());
        diagnosticsModel.incrementMetric(QStringLiteral("navigation.routeChangeCount"));
        diagnosticsModel.recordEvent(QStringLiteral("navigation"),
                                     QStringLiteral("routeChanged"),
                                     {{QStringLiteral("route"), quickRuntime.currentRoute()},
                                      {QStringLiteral("depth"), quickRuntime.depth()}},
                                     QStringLiteral("info"),
                                     quickRuntime.currentRoute());
    });
    QObject::connect(&quickRuntime, &QuickRuntime::navigationChanged,
                     &diagnosticsModel, [&] {
        diagnosticsModel.setMetric(QStringLiteral("runtime.depth"), quickRuntime.depth());
        diagnosticsModel.setMetric(QStringLiteral("runtime.canGoBack"), quickRuntime.canGoBack());
    });
    QObject::connect(&quickRuntime, &QuickRuntime::cacheStateChanged,
                     &diagnosticsModel, [&] {
        diagnosticsModel.setMetric(QStringLiteral("runtime.singletonCount"), quickRuntime.singletonCount());
        diagnosticsModel.setMetric(QStringLiteral("runtime.retainedLruWeight"), quickRuntime.retainedLruWeight());
    });
    QObject::connect(&quickRuntime, &QuickRuntime::cacheEntryEvicted,
                     &diagnosticsModel, [&](const QString &route) {
        diagnosticsModel.incrementMetric(QStringLiteral("runtime.cacheEvictionCount"));
        diagnosticsModel.recordEvent(QStringLiteral("cache"),
                                     QStringLiteral("evicted"),
                                     {{QStringLiteral("route"), route}},
                                     QStringLiteral("info"),
                                     route);
    });
    QObject::connect(&quickRuntime, &QuickRuntime::navigationBlocked,
                     &diagnosticsModel,
                     [&](const QString &fromRoute, const QString &toRoute, const QString &reason) {
        diagnosticsModel.recordEvent(QStringLiteral("navigation"),
                                     QStringLiteral("blocked"),
                                     {{QStringLiteral("fromRoute"), fromRoute},
                                      {QStringLiteral("toRoute"), toRoute},
                                      {QStringLiteral("reason"), reason}},
                                     QStringLiteral("warning"),
                                     reason);
    });
    QObject::connect(&quickRuntime, &QuickRuntime::navigationFailed,
                     &diagnosticsModel, [&](const QString &route, const QString &message) {
        diagnosticsModel.recordEvent(QStringLiteral("navigation"),
                                     QStringLiteral("failed"),
                                     {{QStringLiteral("route"), route}},
                                     QStringLiteral("error"),
                                     message);
    });
    QObject::connect(&quickRuntime, &QuickRuntime::pageFallbackAttempted,
                     &diagnosticsModel,
                     [&](const QString &route, const QString &primaryProvider,
                         const QString &fallbackProvider, const QString &primaryError) {
        diagnosticsModel.recordEvent(QStringLiteral("fallback"),
                                     QStringLiteral("attempted"),
                                     {{QStringLiteral("route"), route},
                                      {QStringLiteral("primaryProvider"), primaryProvider},
                                      {QStringLiteral("fallbackProvider"), fallbackProvider},
                                      {QStringLiteral("primaryError"), primaryError}},
                                     QStringLiteral("warning"),
                                     route);
    });
    QObject::connect(&quickRuntime, &QuickRuntime::pageFallbackActivated,
                     &diagnosticsModel,
                     [&](const QString &route, const QString &provider, const QString &strategy) {
        diagnosticsModel.incrementMetric(QStringLiteral("fallback.activatedCount"));
        diagnosticsModel.recordEvent(QStringLiteral("fallback"),
                                     QStringLiteral("activated"),
                                     {{QStringLiteral("route"), route},
                                      {QStringLiteral("provider"), provider},
                                      {QStringLiteral("strategy"), strategy}},
                                     QStringLiteral("info"),
                                     strategy);
    });
    QObject::connect(&quickRuntime, &QuickRuntime::pageFallbackFailed,
                     &diagnosticsModel,
                     [&](const QString &route, const QString &provider, const QString &message) {
        diagnosticsModel.recordEvent(QStringLiteral("fallback"),
                                     QStringLiteral("failed"),
                                     {{QStringLiteral("route"), route},
                                      {QStringLiteral("provider"), provider}},
                                     QStringLiteral("error"),
                                     message);
    });
    QObject::connect(quickRuntime.errorCollector(), &QmlErrorCollector::errorRecorded,
                     &diagnosticsModel, [&](const QVariantMap &entry) {
        diagnosticsModel.incrementMetric(QStringLiteral("qml.errorCount"));
        diagnosticsModel.recordEvent(QStringLiteral("qml"),
                                     QStringLiteral("error"),
                                     entry,
                                     QStringLiteral("error"),
                                     entry.value(QStringLiteral("message")).toString());
    });
#endif

    engine.rootContext()->setContextProperty(QStringLiteral("sampleModel"), &sampleModel);
    engine.rootContext()->setContextProperty(QStringLiteral("resultModel"), &resultModel);
    engine.rootContext()->setContextProperty(QStringLiteral("scanner"), &scanner);
    engine.rootContext()->setContextProperty(QStringLiteral("inputRouter"), &inputRouter);
    engine.rootContext()->setContextProperty(QStringLiteral("overlayCoordinator"), &overlayCoordinator);
    engine.rootContext()->setContextProperty(QStringLiteral("permissionEvaluator"), &permissionEvaluator);
    engine.rootContext()->setContextProperty(QStringLiteral("quickRuntime"), &quickRuntime);
#ifdef LFQUICK_HAS_QT_GRAPHS_ADAPTER
    engine.rootContext()->setContextProperty(QStringLiteral("chartsAvailable"), true);
    engine.rootContext()->setContextProperty(QStringLiteral("syntheticChartModel"), &syntheticChartModel);
#else
    engine.rootContext()->setContextProperty(QStringLiteral("chartsAvailable"), false);
#endif
#ifdef LFQUICK_HAS_DIAGNOSTICS
    engine.rootContext()->setContextProperty(QStringLiteral("diagnosticsAvailable"), true);
    engine.rootContext()->setContextProperty(QStringLiteral("diagnosticsModel"), &diagnosticsModel);
#else
    engine.rootContext()->setContextProperty(QStringLiteral("diagnosticsAvailable"), false);
    engine.rootContext()->setContextProperty(QStringLiteral("diagnosticsModel"), static_cast<QObject *>(nullptr));
#endif

    QTimer liveDataTimer(&app);
    QObject::connect(&liveDataTimer, &QTimer::timeout, &sampleModel, [&sampleModel
#ifdef LFQUICK_HAS_DIAGNOSTICS
                                                                     , &diagnosticsModel
#endif
    ] {
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
#ifdef LFQUICK_HAS_DIAGNOSTICS
        diagnosticsModel.incrementMetric(QStringLiteral("dataview.liveMutationCount"), 3);
#endif
    });
    liveDataTimer.start(800);

    QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed,
                     &app, [] { QCoreApplication::exit(-1); },
                     Qt::QueuedConnection);
    engine.loadFromModule(QStringLiteral("LfQuick.MedicalDemo"),
                          QStringLiteral("App"));

    return app.exec();
}
