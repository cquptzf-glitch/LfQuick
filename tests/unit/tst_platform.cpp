#include <LfQuick/Platform/ClipboardService.h>
#include <LfQuick/Platform/FileWatcher.h>
#include <LfQuick/Platform/InputRouter.h>
#include <LfQuick/Platform/InputTarget.h>
#include <LfQuick/Platform/KeyboardWedgeScanner.h>
#include <LfQuick/Platform/RuntimeCapabilities.h>
#include <LfQuick/Platform/ScannerSession.h>
#include <LfQuick/Platform/ScreenService.h>
#include <LfQuick/Platform/SystemAppearance.h>

#include <QClipboard>
#include <QDir>
#include <QFile>
#include <QGuiApplication>
#include <QKeyEvent>
#include <QSaveFile>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTest>

using namespace LfQuick;

class PlatformTest : public QObject
{
    Q_OBJECT

private slots:
    void sessionAcceptsPrefixAndSuffix()
    {
        ScannerSession session;
        session.setMinimumLength(4);
        session.setPrefix(QStringLiteral("["));
        session.setSuffix(QStringLiteral("]"));

        QSignalSpy accepted(&session, &ScannerSession::scanAccepted);
        QSignalSpy rejected(&session, &ScannerSession::scanRejected);

        session.feed(QStringLiteral("[S001]"));
        QVERIFY(session.finish());
        QCOMPARE(accepted.count(), 1);
        QCOMPARE(accepted.at(0).at(0).toString(), QStringLiteral("S001"));
        QCOMPARE(rejected.count(), 0);
    }

    void wedgeConsumesScannerSequence()
    {
        KeyboardWedgeScanner scanner;
        scanner.setEnabled(true);
        scanner.session()->setMinimumLength(4);

        QSignalSpy accepted(scanner.session(), &ScannerSession::scanAccepted);

        const QString text = QStringLiteral("S001");
        for (const auto ch : text) {
            QKeyEvent event(QEvent::KeyPress, ch.unicode(), Qt::NoModifier, QString(ch));
            QVERIFY(scanner.handleKeyEvent(&event));
        }

        QKeyEvent enter(QEvent::KeyPress, Qt::Key_Enter, Qt::NoModifier, QString());
        QVERIFY(scanner.handleKeyEvent(&enter));
        QCOMPARE(accepted.count(), 1);
    }

    void inputTargetNotifiesWhenBoundObjectIsDestroyed()
    {
        InputTarget inputTarget;
        auto *target = new QObject;
        inputTarget.setTarget(target);
        QCOMPARE(inputTarget.target(), target);

        QSignalSpy changed(&inputTarget, &InputTarget::targetChanged);
        delete target;

        QCOMPARE(inputTarget.target(), nullptr);
        QCOMPARE(changed.count(), 1);
    }

    void inputRouterTracksScannerDestruction()
    {
        InputRouter router;
        QSignalSpy scannerChanged(&router, &InputRouter::scannerChanged);
        QSignalSpy sourcesChanged(&router, &InputRouter::sourcesChanged);

        auto *scanner = new KeyboardWedgeScanner;
        router.setScanner(scanner);
        QCOMPARE(router.scanner(), scanner);
        QCOMPARE(router.sourceCount(), 1);

        scannerChanged.clear();
        sourcesChanged.clear();
        delete scanner;

        QCOMPARE(router.scanner(), nullptr);
        QCOMPARE(router.sourceCount(), 0);
        QCOMPARE(scannerChanged.count(), 1);
        QCOMPARE(sourcesChanged.count(), 1);
    }

    void inputRouterClearSourcesClearsCompatibilityScanner()
    {
        InputRouter router;
        KeyboardWedgeScanner scanner;
        router.setScanner(&scanner);
        QCOMPARE(router.sourceCount(), 1);

        QSignalSpy scannerChanged(&router, &InputRouter::scannerChanged);
        router.clearSources();

        QCOMPARE(router.scanner(), nullptr);
        QCOMPARE(router.sourceCount(), 0);
        QCOMPARE(scannerChanged.count(), 1);
    }

    void inputRouterTracksInstallTargetDestruction()
    {
        InputRouter router;
        auto *target = new QObject;
        QVERIFY(router.installOn(target));
        QVERIFY(router.installed());

        QSignalSpy installedChanged(&router, &InputRouter::installedChanged);
        delete target;

        QVERIFY(!router.installed());
        QCOMPARE(installedChanged.count(), 1);
    }

    void inputRouterTargetRegistrationDoesNotAccumulateDestroyedCallbacks()
    {
        InputRouter router;
        auto *target = new InputTarget;

        for (int i = 0; i < 3; ++i) {
            QVERIFY(router.registerTarget(target));
            QCOMPARE(router.targetCount(), 1);
            QVERIFY(router.unregisterTarget(target));
            QCOMPARE(router.targetCount(), 0);
        }
        QVERIFY(router.registerTarget(target));

        QSignalSpy targetsChanged(&router, &InputRouter::targetsChanged);
        delete target;

        QCOMPARE(router.targetCount(), 0);
        QCOMPARE(targetsChanged.count(), 1);
    }

    void runtimeCapabilitiesExposeStableKeys()
    {
        RuntimeCapabilities capabilities;
        QCOMPARE(capabilities.qtVersion(), QString::fromLatin1(qVersion()));

        const QVariantMap values = capabilities.capabilities();
        const QStringList stableKeys{
            QStringLiteral("module.io"),
            QStringLiteral("module.models"),
            QStringLiteral("module.ui"),
            QStringLiteral("module.dataView"),
            QStringLiteral("module.platformQml"),
            QStringLiteral("module.integration"),
            QStringLiteral("module.widgetBridge"),
            QStringLiteral("module.charts"),
            QStringLiteral("module.diagnostics"),
            QStringLiteral("adapter.qtGraphs"),
            QStringLiteral("adapter.rangeModel"),
            QStringLiteral("adapter.canvasPainter"),
            QStringLiteral("adapter.safeArea"),
            QStringLiteral("adapter.customQuickCharts"),
            QStringLiteral("adapter.legacyQCustomPlot"),
            QStringLiteral("adapter.styleKit"),
            QStringLiteral("style.styleKit"),
            QStringLiteral("style.fluent"),
            QStringLiteral("style.brandTest"),
            QStringLiteral("tooling.hotReload"),
            QStringLiteral("tooling.screenshot"),
            QStringLiteral("qt.supportsSafeArea"),
            QStringLiteral("qt.supportsRangeModel"),
            QStringLiteral("qt.supportsCanvasPainter"),
            QStringLiteral("qt.supportsCanvas2D"),
            QStringLiteral("qt.supportsQmlPreview"),
            QStringLiteral("qt.supportsStyleKit")
        };
        for (const QString &key : stableKeys)
            QVERIFY2(values.contains(key), qPrintable(QStringLiteral("Missing capability key: %1").arg(key)));

        // Platform's public FileWatcher compatibility API requires Io, so any
        // successfully configured Platform build must report this as enabled.
        QVERIFY(capabilities.hasCapability(QStringLiteral("module.io")));
        QCOMPARE(capabilities.hasCapability(QStringLiteral("qt.supportsRangeModel")),
                 QT_VERSION >= QT_VERSION_CHECK(6, 10, 0));
        QCOMPARE(capabilities.hasCapability(QStringLiteral("qt.supportsCanvas2D")),
                 QT_VERSION >= QT_VERSION_CHECK(6, 12, 0));
        QVERIFY(!capabilities.hasCapability(QStringLiteral("missing.capability")));
    }

    void clipboardServiceRoundTripsText()
    {
        QClipboard *clipboard = QGuiApplication::clipboard();
        if (!clipboard)
            QSKIP("Clipboard is unavailable on this platform plugin");

        const QString previous = clipboard->text();
        ClipboardService service;
        QSignalSpy changed(&service, &ClipboardService::textChanged);

        const QString value = QStringLiteral("LfQuick clipboard test");
        service.copyText(value);
        QCOMPARE(service.text(), value);
        QVERIFY(service.hasText());
        QVERIFY(changed.count() >= 1);

        service.clear();
        QVERIFY(!service.hasText());
        service.copyText(previous);
    }

    void screenServiceReturnsPrimaryMetadata()
    {
        ScreenService service;
        if (service.count() <= 0)
            QSKIP("No screen is exposed by this platform plugin");

        const QVariantMap primary = service.primaryScreen();
        QVERIFY(!primary.isEmpty());
        QVERIFY(primary.contains(QStringLiteral("name")));
        QVERIFY(primary.contains(QStringLiteral("geometry")));
        QVERIFY(primary.contains(QStringLiteral("availableGeometry")));
        QVERIFY(primary.value(QStringLiteral("devicePixelRatio")).toDouble() > 0.0);

        const QVariantMap geometry = primary.value(QStringLiteral("geometry")).toMap();
        QVERIFY(geometry.value(QStringLiteral("width")).toInt() > 0);
        QVERIFY(geometry.value(QStringLiteral("height")).toInt() > 0);

        SystemAppearance appearance;
        QVERIFY(!appearance.colorSchemeName().isEmpty());
    }

    void fileWatcherPathKindsStayMutuallyExclusive()
    {
        QTemporaryDir temporary;
        QVERIFY(temporary.isValid());

        FileWatcher watcher;
        const QString directoryPath = temporary.filePath(QStringLiteral("kind-switch"));
        QVERIFY(QDir().mkpath(directoryPath));
        QVERIFY(!watcher.watchFile(directoryPath));
        QCOMPARE(watcher.count(), 0);

        const QString missingFile = temporary.filePath(QStringLiteral("future.json"));
        QVERIFY(watcher.watchFile(missingFile));
        QCOMPARE(watcher.count(), 1);
        QVERIFY(watcher.watchedFiles().contains(QDir::cleanPath(missingFile)));
        QVERIFY(watcher.unwatch(missingFile));
        QCOMPARE(watcher.count(), 0);

        QVERIFY(QDir(directoryPath).removeRecursively());
        {
            QFile file(directoryPath);
            QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Truncate));
            QCOMPARE(file.write("file"), qint64(4));
        }
        QVERIFY(watcher.watchFile(directoryPath));
        QCOMPARE(watcher.count(), 1);
        QVERIFY(watcher.watchedFiles().contains(QDir::cleanPath(directoryPath)));

        QVERIFY(QFile::remove(directoryPath));
        QVERIFY(QDir().mkpath(directoryPath));
        QVERIFY(watcher.watchDirectory(directoryPath));
        QCOMPARE(watcher.count(), 1);
        QVERIFY(!watcher.watchedFiles().contains(QDir::cleanPath(directoryPath)));
        QVERIFY(watcher.watchedDirectories().contains(QDir::cleanPath(directoryPath)));

        QVERIFY(watcher.unwatch(directoryPath));
        QCOMPARE(watcher.count(), 0);
    }

    void fileWatcherIgnoresUnrelatedSiblingChanges()
    {
        QTemporaryDir temporary;
        QVERIFY(temporary.isValid());
        const QString watchedPath = temporary.filePath(QStringLiteral("settings.json"));
        const QString siblingPath = temporary.filePath(QStringLiteral("unrelated.log"));

        {
            QFile file(watchedPath);
            QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Truncate));
            QCOMPARE(file.write("one"), qint64(3));
        }

        FileWatcher watcher;
        QVERIFY(watcher.watchFile(watchedPath));
        QSignalSpy changed(&watcher, &FileWatcher::fileChanged);

        {
            QFile sibling(siblingPath);
            QVERIFY(sibling.open(QIODevice::WriteOnly | QIODevice::Truncate));
            QCOMPARE(sibling.write("noise"), qint64(5));
        }
        QTest::qWait(150);
        QCOMPARE(changed.count(), 0);

        {
            QFile file(watchedPath);
            QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Truncate));
            QCOMPARE(file.write("two-two"), qint64(7));
            file.flush();
        }
        QTRY_VERIFY_WITH_TIMEOUT(changed.count() > 0, 1500);
    }

    void fileWatcherSurvivesAtomicReplacement()
    {
        QTemporaryDir temporary;
        QVERIFY(temporary.isValid());
        const QString path = temporary.filePath(QStringLiteral("settings.json"));

        {
            QFile file(path);
            QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Truncate));
            QCOMPARE(file.write("one"), qint64(3));
        }

        FileWatcher watcher;
        QVERIFY(watcher.watchFile(path));
        QCOMPARE(watcher.count(), 1);
        QVERIFY(watcher.isWatching(path));
        QSignalSpy changed(&watcher, &FileWatcher::fileChanged);

        {
            QSaveFile file(path);
            QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Truncate));
            QCOMPARE(file.write("two"), qint64(3));
            QVERIFY(file.commit());
        }

        QTRY_VERIFY_WITH_TIMEOUT(changed.count() > 0, 1500);
        QVERIFY(watcher.isWatching(path));
        QVERIFY(watcher.unwatch(path));
        QCOMPARE(watcher.count(), 0);
    }
};

QTEST_MAIN(PlatformTest)
#include "tst_platform.moc"
