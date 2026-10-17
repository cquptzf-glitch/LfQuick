#pragma once

#include <LfQuick/Ui/FocusWalker.h>
#include <LfQuick/Ui/LfQuickUiGlobal.h>

#include <QObject>
#include <QPointer>
#include <QQuickItem>
#include <QStringList>
#include <QtQmlIntegration/qqmlintegration.h>

namespace LfQuick {

class LFQUICKUI_EXPORT FocusManager : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(QQuickItem* scope READ scope WRITE setScope NOTIFY scopeChanged FINAL)
    Q_PROPERTY(QQuickItem* currentFocus READ currentFocus NOTIFY currentFocusChanged FINAL)
    Q_PROPERTY(FocusWalker* walker READ walker CONSTANT FINAL)
    Q_PROPERTY(FocusWalker::Mode mode READ mode WRITE setMode NOTIFY policyChanged FINAL)
    Q_PROPERTY(bool wrap READ wrap WRITE setWrap NOTIFY policyChanged FINAL)

public:
    explicit FocusManager(QObject *parent = nullptr);

    QQuickItem *scope() const noexcept { return m_scope; }
    void setScope(QQuickItem *scope);

    QQuickItem *currentFocus() const;
    FocusWalker *walker() noexcept { return &m_walker; }

    FocusWalker::Mode mode() const noexcept { return m_mode; }
    void setMode(FocusWalker::Mode mode);

    bool wrap() const noexcept { return m_wrap; }
    void setWrap(bool wrap);

    Q_INVOKABLE bool focus(QQuickItem *item, int reason = int(Qt::OtherFocusReason));
    Q_INVOKABLE bool moveNext();
    Q_INVOKABLE bool movePrevious();
    Q_INVOKABLE bool moveLeft();
    Q_INVOKABLE bool moveRight();
    Q_INVOKABLE bool moveUp();
    Q_INVOKABLE bool moveDown();
    Q_INVOKABLE QVariantList dumpOrder() const;
    Q_INVOKABLE QStringList focusChain() const;

signals:
    void scopeChanged();
    void currentFocusChanged();
    void policyChanged();
    void focusMoved(QQuickItem *item);

private:
    bool moveLinear(bool forward);
    bool moveDirectional(FocusWalker::Direction direction);
    void reconnectWindow();

    QPointer<QQuickItem> m_scope;
    FocusWalker m_walker;
    FocusWalker::Mode m_mode = FocusWalker::Mode::Native;
    bool m_wrap = true;
    QMetaObject::Connection m_scopeWindowConnection;
    QMetaObject::Connection m_focusConnection;
    QMetaObject::Connection m_windowDestroyedConnection;
};

} // namespace LfQuick
