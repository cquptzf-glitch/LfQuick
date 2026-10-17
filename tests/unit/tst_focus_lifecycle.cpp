#include <LfQuick/Ui/FocusManager.h>
#include <LfQuick/Ui/FocusNavigator.h>
#include <LfQuick/Ui/FocusWalker.h>

#include <QQuickItem>
#include <QSignalSpy>
#include <QTest>

using namespace LfQuick;

class FocusLifecycleTest : public QObject
{
    Q_OBJECT

private slots:
    void walkerNotifiesWhenScopeIsDestroyed()
    {
        FocusWalker walker;
        auto *scope = new QQuickItem;
        walker.setScope(scope);
        QCOMPARE(walker.scope(), scope);

        QSignalSpy changed(&walker, &FocusWalker::scopeChanged);
        delete scope;

        QCOMPARE(walker.scope(), nullptr);
        QCOMPARE(changed.count(), 1);
    }

    void managerNotifiesWhenScopeIsDestroyed()
    {
        FocusManager manager;
        auto *scope = new QQuickItem;
        manager.setScope(scope);
        QCOMPARE(manager.scope(), scope);

        QSignalSpy scopeChanged(&manager, &FocusManager::scopeChanged);
        QSignalSpy focusChanged(&manager, &FocusManager::currentFocusChanged);
        delete scope;

        QCOMPARE(manager.scope(), nullptr);
        QCOMPARE(manager.currentFocus(), nullptr);
        QCOMPARE(scopeChanged.count(), 1);
        QCOMPARE(focusChanged.count(), 1);
    }

    void navigatorNotifiesWhenScopeIsDestroyed()
    {
        FocusNavigator navigator;
        auto *scope = new QQuickItem;
        navigator.setScope(scope);
        QCOMPARE(navigator.scope(), scope);

        QSignalSpy scopeChanged(&navigator, &FocusNavigator::scopeChanged);
        QSignalSpy focusChanged(&navigator, &FocusNavigator::currentFocusChanged);
        delete scope;

        QCOMPARE(navigator.scope(), nullptr);
        QCOMPARE(navigator.currentFocus(), nullptr);
        QCOMPARE(scopeChanged.count(), 1);
        QCOMPARE(focusChanged.count(), 1);
    }
};

QTEST_MAIN(FocusLifecycleTest)
#include "tst_focus_lifecycle.moc"
