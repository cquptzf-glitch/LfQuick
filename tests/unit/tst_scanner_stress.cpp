#include <LfQuick/Platform/KeyboardWedgeScanner.h>
#include <LfQuick/Platform/ScannerSession.h>

#include <QKeyEvent>
#include <QSignalSpy>
#include <QTest>

using namespace LfQuick;

class ScannerStressTest : public QObject
{
    Q_OBJECT

private slots:
    void keyboardWedgeDeliversOneThousandScansWithoutLoss()
    {
        KeyboardWedgeScanner scanner;
        scanner.setEnabled(true);
        scanner.session()->setMinimumLength(4);

        QSignalSpy accepted(scanner.session(), &ScannerSession::scanAccepted);
        QSignalSpy rejected(scanner.session(), &ScannerSession::scanRejected);

        constexpr int scanCount = 1000;
        for (int scan = 0; scan < scanCount; ++scan) {
            const QString value = QStringLiteral("S%1").arg(scan, 4, 10, QLatin1Char('0'));
            for (const QChar ch : value) {
                QKeyEvent event(QEvent::KeyPress,
                                ch.unicode(),
                                Qt::NoModifier,
                                QString(ch));
                QVERIFY2(scanner.handleKeyEvent(&event),
                         qPrintable(QStringLiteral("character rejected for scan %1").arg(scan)));
            }

            QKeyEvent enter(QEvent::KeyPress, Qt::Key_Enter, Qt::NoModifier, QString());
            QVERIFY2(scanner.handleKeyEvent(&enter),
                     qPrintable(QStringLiteral("terminator rejected for scan %1").arg(scan)));
        }

        QCOMPARE(accepted.count(), scanCount);
        QCOMPARE(rejected.count(), 0);
        QCOMPARE(accepted.first().at(0).toString(), QStringLiteral("S0000"));
        QCOMPARE(accepted.last().at(0).toString(), QStringLiteral("S0999"));
    }
};

QTEST_MAIN(ScannerStressTest)
#include "tst_scanner_stress.moc"
