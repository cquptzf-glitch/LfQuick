#include <LfQuick/Runtime/PageDescriptor.h>
#include <LfQuick/Ui/OverlayCoordinator.h>
#include <LfQuick/Ui/QuickRuntime.h>
#include <LfQuick/WidgetBridge/QuickSurface.h>
#include <LfQuick/WidgetBridge/WidgetFocusBridge.h>
#include <LfQuick/WidgetBridge/WidgetOverlayBackend.h>
#include <LfQuick/WidgetBridge/WidgetPageProvider.h>

#include <QApplication>
#include <QCheckBox>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QQmlEngine>
#include <QQuickItem>
#include <QStackedWidget>
#include <QTableWidget>
#include <QVariant>
#include <QVBoxLayout>
#include <QWidget>

#include <memory>

using namespace LfQuick;

namespace {

QWidget *createLegacyResultsPage(QWidget *parent, const QVariantMap &parameters)
{
    auto *page = new QWidget(parent);
    page->setObjectName(QStringLiteral("legacyResultsPage"));

    auto *layout = new QVBoxLayout(page);
    layout->setContentsMargins(24, 24, 24, 24);
    layout->setSpacing(12);

    auto *title = new QLabel(parameters.value(QStringLiteral("title"),
                                               QStringLiteral("Legacy QWidget Results"))
                                 .toString(), page);
    QFont titleFont = title->font();
    titleFont.setPointSize(titleFont.pointSize() + 6);
    titleFont.setBold(true);
    title->setFont(titleFont);
    layout->addWidget(title);

    auto *description = new QLabel(
        QStringLiteral("This is a legacy QWidget page created through WidgetPageProvider. "
                       "It shares PageRouter history with the QML page and can also be "
                       "selected automatically when the QML implementation fails."), page);
    description->setWordWrap(true);
    layout->addWidget(description);

    auto *table = new QTableWidget(6, 4, page);
    table->setHorizontalHeaderLabels({QStringLiteral("Sample"), QStringLiteral("Assay"),
                                      QStringLiteral("Result"), QStringLiteral("Status")});
    table->horizontalHeader()->setStretchLastSection(true);
    for (int row = 0; row < table->rowCount(); ++row) {
        table->setItem(row, 0, new QTableWidgetItem(QStringLiteral("S-%1").arg(row + 1, 4, 10, QLatin1Char('0'))));
        table->setItem(row, 1, new QTableWidgetItem(row % 2 ? QStringLiteral("ALT") : QStringLiteral("GLU")));
        table->setItem(row, 2, new QTableWidgetItem(QString::number(3.5 + row * 0.7, 'f', 2)));
        table->setItem(row, 3, new QTableWidgetItem(row == 4 ? QStringLiteral("Review") : QStringLiteral("Completed")));
    }
    layout->addWidget(table, 1);

    return page;
}

} // namespace

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QWidget window;
    window.setWindowTitle(QStringLiteral("LfQuick Hybrid Migration Demo"));
    window.resize(1200, 820);

    OverlayCoordinator overlayCoordinator;
    WidgetOverlayBackend overlayBackend(&window);
    overlayCoordinator.setBackend(&overlayBackend);

    auto *rootLayout = new QVBoxLayout(&window);
    auto *toolbar = new QHBoxLayout;
    auto *quickButton = new QPushButton(QStringLiteral("QML Page"), &window);
    auto *legacyButton = new QPushButton(QStringLiteral("Legacy QWidget Page"), &window);
    auto *fallbackButton = new QPushButton(QStringLiteral("Broken QML → Legacy Fallback"), &window);
    auto *backButton = new QPushButton(QStringLiteral("Back"), &window);
    auto *toastButton = new QPushButton(QStringLiteral("Toast"), &window);
    auto *dialogButton = new QPushButton(QStringLiteral("Dialog"), &window);
    auto *routeLabel = new QLabel(QStringLiteral("Route: -"), &window);
    auto *metricsLabel = new QLabel(QStringLiteral("QuickSurface: loading"), &window);

    quickButton->setEnabled(false);
    legacyButton->setEnabled(false);
    fallbackButton->setEnabled(false);
    backButton->setEnabled(false);

    toolbar->addWidget(quickButton);
    toolbar->addWidget(legacyButton);
    toolbar->addWidget(fallbackButton);
    toolbar->addWidget(backButton);
    toolbar->addWidget(toastButton);
    toolbar->addWidget(dialogButton);
    toolbar->addSpacing(16);
    toolbar->addWidget(routeLabel);
    toolbar->addStretch(1);
    toolbar->addWidget(metricsLabel);
    rootLayout->addLayout(toolbar);

    auto *focusRow = new QHBoxLayout;
    auto *beforeFocusEdit = new QLineEdit(&window);
    beforeFocusEdit->setPlaceholderText(QStringLiteral("QWidget before QML (Tab →)"));
    auto *afterFocusEdit = new QLineEdit(&window);
    afterFocusEdit->setPlaceholderText(QStringLiteral("QWidget after QML (Shift+Tab ←)"));
    auto *scannerIsolation = new QCheckBox(QStringLiteral("Scanner focus isolation"), &window);
    focusRow->addWidget(beforeFocusEdit);
    focusRow->addWidget(new QLabel(QStringLiteral("→ QML TextField →"), &window));
    focusRow->addWidget(afterFocusEdit);
    focusRow->addWidget(scannerIsolation);
    rootLayout->addLayout(focusRow);

    auto *stack = new QStackedWidget(&window);
    rootLayout->addWidget(stack, 1);

    QQmlEngine engine;
    auto *quickSurface = new QuickSurface(&engine, stack);
    stack->addWidget(quickSurface);
    WidgetFocusBridge focusBridge(quickSurface, &window);
    focusBridge.setBoundaryWidgets(beforeFocusEdit, afterFocusEdit);

    QObject::connect(scannerIsolation, &QCheckBox::toggled,
                     &focusBridge, &WidgetFocusBridge::setScannerIsolation);

    QuickRuntime runtime(&engine);
    runtime.setQuickActivationCallback([stack, quickSurface, &focusBridge] {
        focusBridge.rememberFocus();
        stack->setCurrentWidget(quickSurface);
        focusBridge.restoreFocus();
    });

    QVariantMap quickPageParameters;
    quickPageParameters.insert(
        QStringLiteral("navigationGuard"),
        QVariant::fromValue(static_cast<QObject *>(runtime.navigationGuard())));

    auto widgetProvider = std::make_shared<WidgetPageProvider>(stack);
    widgetProvider->registerFactory(QStringLiteral("legacy-results"), createLegacyResultsPage);
    runtime.registerProvider(widgetProvider);

    PageDescriptor legacyDescriptor;
    legacyDescriptor.route = QStringLiteral("/legacy/results");
    legacyDescriptor.provider = QStringLiteral("widget");
    legacyDescriptor.source.insert(QStringLiteral("factory"), QStringLiteral("legacy-results"));
    legacyDescriptor.cachePolicy = PageCachePolicy::KeepAlive;
    runtime.registerDescriptor(legacyDescriptor);

    bool routesInitialized = false;
    const auto initializeQuickRoutes = [&] {
        if (routesInitialized || !quickSurface->isReady())
            return;

        auto *hostItem = qobject_cast<QQuickItem *>(quickSurface->rootObject());
        if (!hostItem) {
            metricsLabel->setText(QStringLiteral("QuickSurface: root is not QQuickItem"));
            return;
        }

        runtime.setHost(hostItem);
        const bool registered = runtime.registerPage(
            QStringLiteral("/quick"),
            QUrl(QStringLiteral("qrc:/qt/qml/LfQuick/HybridDemo/HybridQuickPage.qml")),
            static_cast<int>(PageCachePolicy::KeepAlive));

        PageDescriptor fallbackDescriptor;
        fallbackDescriptor.route = QStringLiteral("/quick/fallback");
        fallbackDescriptor.provider = QStringLiteral("quick");
        fallbackDescriptor.source.insert(
            QStringLiteral("url"),
            QUrl(QStringLiteral("qrc:/qt/qml/LfQuick/HybridDemo/MissingHybridQuickPage.qml")));
        fallbackDescriptor.source.insert(
            QStringLiteral("fallbackUrl"),
            QUrl(QStringLiteral("qrc:/qt/qml/LfQuick/PageLoadError.qml")));
        fallbackDescriptor.cachePolicy = PageCachePolicy::KeepAlive;
        fallbackDescriptor.fallbackPolicy = FallbackPolicy::LegacyBeforeActivate;
        PageFallbackDescriptor legacyFallback;
        legacyFallback.provider = QStringLiteral("widget");
        legacyFallback.source.insert(QStringLiteral("factory"), QStringLiteral("legacy-results"));
        fallbackDescriptor.fallback = std::move(legacyFallback);

        if (!registered || !runtime.registerDescriptor(std::move(fallbackDescriptor))) {
            metricsLabel->setText(QStringLiteral("QuickSurface: route registration failed"));
            return;
        }

        routesInitialized = true;
        quickButton->setEnabled(true);
        legacyButton->setEnabled(true);
        fallbackButton->setEnabled(true);
        runtime.push(QStringLiteral("/quick"), quickPageParameters);

        if (auto *focusItem = hostItem->findChild<QQuickItem *>(QStringLiteral("hybridFocusField")))
            focusBridge.setQuickBoundaryItems(focusItem, focusItem);
    };

    QObject::connect(quickSurface, &QuickSurface::readyChanged,
                     &window, initializeQuickRoutes);
    QObject::connect(quickSurface, &QuickSurface::firstFrameRendered,
                     &window, [quickSurface, metricsLabel](qint64 firstFrameMs) {
        metricsLabel->setText(
            QStringLiteral("QuickSurface load=%1 ms, first-frame=%2 ms")
                .arg(quickSurface->loadDurationMs())
                .arg(firstFrameMs));
    });
    QObject::connect(quickSurface, &QuickSurface::loadFailed,
                     &window, [metricsLabel](const QString &message) {
        metricsLabel->setText(QStringLiteral("QuickSurface error: %1").arg(message));
    });
    QObject::connect(&focusBridge, &WidgetFocusBridge::focusTransferred,
                     &window, [metricsLabel](const QString &direction) {
        metricsLabel->setText(QStringLiteral("Focus bridge: %1").arg(direction));
    });
    QObject::connect(&runtime, &QuickRuntime::pageFallbackActivated,
                     &window, [metricsLabel](const QString &route,
                                             const QString &provider,
                                             const QString &strategy) {
        metricsLabel->setText(
            QStringLiteral("Fallback %1: %2 via %3").arg(strategy, route, provider));
    });
    QObject::connect(&runtime, &QuickRuntime::pageFallbackFailed,
                     &window, [metricsLabel](const QString &route,
                                             const QString &provider,
                                             const QString &message) {
        metricsLabel->setText(
            QStringLiteral("Fallback failed: %1 via %2 — %3").arg(route, provider, message));
    });

    QObject::connect(quickButton, &QPushButton::clicked,
                     &window, [&runtime, quickPageParameters] {
        runtime.push(QStringLiteral("/quick"), quickPageParameters);
    });
    QObject::connect(legacyButton, &QPushButton::clicked,
                     &window, [&runtime, &focusBridge] {
        focusBridge.rememberFocus();
        runtime.push(QStringLiteral("/legacy/results"),
                     {{QStringLiteral("title"), QStringLiteral("Legacy Biochemistry Results")}});
    });
    QObject::connect(fallbackButton, &QPushButton::clicked,
                     &window, [&runtime, &focusBridge] {
        focusBridge.rememberFocus();
        runtime.push(QStringLiteral("/quick/fallback"),
                     {{QStringLiteral("title"),
                       QStringLiteral("Legacy fallback after QML creation failure")}});
    });
    QObject::connect(backButton, &QPushButton::clicked,
                     &window, [&runtime, &focusBridge] {
        focusBridge.rememberFocus();
        if (runtime.back())
            focusBridge.restoreFocus();
    });
    QObject::connect(toastButton, &QPushButton::clicked,
                     &window, [&overlayCoordinator] {
        overlayCoordinator.showToast(
            QStringLiteral("Shared coordinator rendered this toast through QWidget"),
            {{QStringLiteral("severity"), QStringLiteral("success")},
             {QStringLiteral("durationMs"), 3500},
             {QStringLiteral("dedupKey"), QStringLiteral("hybrid-toast")}});
    });
    QObject::connect(dialogButton, &QPushButton::clicked,
                     &window, [&overlayCoordinator] {
        overlayCoordinator.requestDialog(
            QStringLiteral("Shared overlay backend"),
            QStringLiteral("This non-blocking dialog uses the same request protocol as QML."),
            {{QStringLiteral("buttons"),
              QStringList{QStringLiteral("yes"), QStringLiteral("no")}},
             {QStringLiteral("defaultButton"), QStringLiteral("no")},
             {QStringLiteral("context"),
              QVariantMap{{QStringLiteral("source"), QStringLiteral("hybrid-demo")}}}});
    });
    QObject::connect(&overlayCoordinator, &OverlayCoordinator::dialogResolved,
                     &window, [metricsLabel](const QString &requestId,
                                             const QString &buttonId,
                                             const QVariantMap &context) {
        Q_UNUSED(requestId)
        metricsLabel->setText(
            QStringLiteral("Dialog result: %1 (%2)")
                .arg(buttonId, context.value(QStringLiteral("source")).toString()));
    });

    const auto updateNavigationUi = [&] {
        routeLabel->setText(QStringLiteral("Route: %1").arg(runtime.currentRoute()));
        backButton->setEnabled(runtime.canGoBack());
    };
    QObject::connect(&runtime, &QuickRuntime::navigationChanged,
                     &window, updateNavigationUi);

    quickSurface->loadHost();
    initializeQuickRoutes();

    window.show();
    return app.exec();
}
