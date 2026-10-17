#include <LfQuick/Platform/HotkeyRouter.h>

#include <QCoreApplication>
#include <QKeyEvent>
#include <QSignalSpy>
#include <QTest>

using namespace LfQuick;

class HotkeyRouterTest : public QObject
{
    Q_OBJECT

private slots:
    void targetDestructionUpdatesInstalledState()
    {
        HotkeyRouter router;
        auto *target = new QObject;

        QVERIFY(router.installOn(target));
        QVERIFY(router.installed());

        QSignalSpy installedChanged(&router, &HotkeyRouter::installedChanged);
        delete target;

        QVERIFY(!router.installed());
        QCOMPARE(installedChanged.count(), 1);
    }

    void reinstallDoesNotLeaveOldDestroyedCallback()
    {
        HotkeyRouter router;
        auto *first = new QObject;
        auto *second = new QObject;

        QVERIFY(router.installOn(first));
        QVERIFY(router.installOn(second));
        QVERIFY(router.installed());

        QSignalSpy installedChanged(&router, &HotkeyRouter::installedChanged);
        delete first;

        QVERIFY(router.installed());
        QCOMPARE(installedChanged.count(), 0);

        delete second;
        QVERIFY(!router.installed());
        QCOMPARE(installedChanged.count(), 1);
    }

    void registeredHotkeyStillTriggersAfterReinstall()
    {
        HotkeyRouter router;
        QObject first;
        QObject second;

        QVERIFY(router.registerHotkey(QStringLiteral("refresh"), QStringLiteral("Ctrl+R")));
        QVERIFY(router.installOn(&first));
        QVERIFY(router.installOn(&second));

        QSignalSpy triggered(&router, &HotkeyRouter::triggered);
        QKeyEvent keyPress(QEvent::KeyPress, Qt::Key_R, Qt::ControlModifier);
        QCoreApplication::sendEvent(&second, &keyPress);

        QCOMPARE(triggered.count(), 1);
        QCOMPARE(triggered.at(0).at(0).toString(), QStringLiteral("refresh"));
    }
};

QTEST_MAIN(HotkeyRouterTest)
#include "tst_hotkey_router.moc"
