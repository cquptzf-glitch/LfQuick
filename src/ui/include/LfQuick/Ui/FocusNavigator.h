#pragma once

#include <LfQuick/Ui/LfQuickUiGlobal.h>

#include <QList>
#include <QMetaObject>
#include <QObject>
#include <QPointer>
#include <QQuickItem>
#include <QStringList>
#include <QtQmlIntegration/qqmlintegration.h>

namespace LfQuick {

class LFQUICKUI_EXPORT FocusNavigator : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(QQuickItem* scope READ scope WRITE setScope NOTIFY scopeChanged FINAL)
    Q_PROPERTY(QQuickItem* currentFocus READ currentFocus NOTIFY currentFocusChanged FINAL)
    Q_PROPERTY(Mode mode READ mode WRITE setMode NOTIFY policyChanged FINAL)
    Q_PROPERTY(bool wrap READ wrap WRITE setWrap NOTIFY policyChanged FINAL)
    Q_PROPERTY(qreal rowTolerance READ rowTolerance WRITE setRowTolerance NOTIFY policyChanged FINAL)

public:
    enum class Mode {
        Native = 0,
        Explicit = 1,
        Geometric = 2,
        Grid = 3
    };
    Q_ENUM(Mode)

    explicit FocusNavigator(QObject *parent = nullptr);

    QQuickItem *scope() const;
    void setScope(QQuickItem *scope);

    QQuickItem *currentFocus() const;

    Mode mode() const noexcept { return m_mode; }
    void setMode(Mode mode);

    bool wrap() const noexcept { return m_wrap; }
    void setWrap(bool wrap);

    qreal rowTolerance() const noexcept { return m_rowTolerance; }
    void setRowTolerance(qreal tolerance);

    Q_INVOKABLE bool moveNext();
    Q_INVOKABLE bool movePrevious();
    Q_INVOKABLE bool moveLeft();
    Q_INVOKABLE bool moveRight();
    Q_INVOKABLE bool moveUp();
    Q_INVOKABLE bool moveDown();
    Q_INVOKABLE QStringList focusChain() const;
    Q_INVOKABLE QVariantList dumpOrder() const;

signals:
    void scopeChanged();
    void currentFocusChanged();
    void policyChanged();

private:
    enum class Direction { Left, Right, Up, Down };

    bool moveLinear(bool forward);
    bool moveDirectional(Direction direction);
    bool focusItem(QQuickItem *item, Qt::FocusReason reason);
    bool isFocusable(QQuickItem *item) const;
    bool isInScope(QQuickItem *item) const;
    QList<QQuickItem *> focusableItems() const;
    QList<QQuickItem *> orderedItems() const;
    QPointF itemCenter(QQuickItem *item) const;
    int explicitOrder(QQuickItem *item) const;
    void reconnectWindow();

    QPointer<QQuickItem> m_scope;
    QMetaObject::Connection m_scopeDestroyedConnection;
    QMetaObject::Connection m_scopeWindowConnection;
    QMetaObject::Connection m_focusConnection;
    QMetaObject::Connection m_windowDestroyedConnection;
    Mode m_mode = Mode::Native;
    bool m_wrap = true;
    qreal m_rowTolerance = 8.0;
};

} // namespace LfQuick
