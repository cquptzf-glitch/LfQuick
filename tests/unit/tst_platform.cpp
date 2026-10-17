#include <LfQuick/Platform/ClipboardService.h>
#include <LfQuick/Platform/FileWatcher.h>
#include <LfQuick/Platform/KeyboardWedgeScanner.h>
#include <LfQuick/Platform/RuntimeCapabilities.h>
#include <LfQuick/Platform/ScannerSession.h>
#include <LfQuick/Platform/ScreenService.h>
#include <LfQuick/Platform/SystemAppearance.h>

#include <QClipboard>
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
        QCOMPARE(accepted.at(0).at(0).toString(), QStringLiteral("S001"));
    }

    void runtimeCapabilitiesExposeStableKeys()
    {
        RuntimeCapabilities capabilities;
        QCOMPARE(capabilities.qtVersion(), QString::fromLatin1(qVersion()));

        const QVariantMap values = capabilities.capabilities();
        QVERIFY(values.contains(QStringLiteral("module.widgetBridge")));
        QVERIFY(values.contains(QStringLiteral("module.charts")));
        QVERIFY(values.contains(QStringLiteral("module.diagnostics")));
        QVERIFY(values.contains(QStringLiteral("tooling.hotReload")));
        QVERIFY(values.contains(QStringLiteral("adapter.styleKit")));
        QVERIFY(values.contains(QStringLiteral("qt.supportsSafeArea")));
        QVERIFY(values.contains(QStringLiteral("qt.supportsRangeModel")));
        QVERIFY(values.contains(QStringLiteral("qt.supportsCanvasPainter")));
        QVERIFY(values.contains(QStringLiteral("qt.supportsQmlPreview")));

        QCOMPARE(capabilities.hasCapability(QStringLiteral("qt.supportsRangeModel")),
                 QT_VERSION >= QT_VERSION_CHECK(6, 10, 0));
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
