#include <LfQuick/Platform/KeyboardWedgeScanner.h>
#include <LfQuick/Platform/ScannerSession.h>

#include <QKeyEvent>
#include <QSignalSpy>
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
};

QTEST_MAIN(PlatformTest)
#include "tst_platform.moc"
