#pragma once

#include <LfQuick/Ui/LfQuickUiGlobal.h>

#include <QMetaObject>
#include <QObject>
#include <QPointer>
#include <QQuickItem>
#include <QVariantList>
#include <QtQmlIntegration/qqmlintegration.h>

namespace LfQuick {

class LFQUICKUI_EXPORT FocusWalker : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(QQuickItem* scope READ scope WRITE setScope NOTIFY scopeChanged FINAL)
    Q_PROPERTY(qreal rowTolerance READ rowTolerance WRITE setRowTolerance NOTIFY policyChanged FINAL)
    Q_PROPERTY(Qt::LayoutDirection layoutDirection READ layoutDirection WRITE setLayoutDirection NOTIFY policyChanged FINAL)

public:
    enum class Mode {
        Native = 0,
        Explicit,
        Geometric,
        Grid
    };
    Q_ENUM(Mode)

    enum class Direction {
        Left = 0,
        Right,
        Up,
        Down
    };
    Q_ENUM(Direction)

    explicit FocusWalker(QObject *parent = nullptr);

    QQuickItem *scope() const noexcept { return m_scope; }
    void setScope(QQuickItem *scope);

    qreal rowTolerance() const noexcept { return m_rowTolerance; }
    void setRowTolerance(qreal value);

    Qt::LayoutDirection layoutDirection() const noexcept { return m_layoutDirection; }
    void setLayoutDirection(Qt::LayoutDirection direction);

    Q_INVOKABLE QQuickItem *next(QQuickItem *current,
                                 Mode mode,
                                 bool forward = true,
                                 bool wrap = true) const;
    Q_INVOKABLE QQuickItem *directional(QQuickItem *current,
                                        Direction direction,
                                        Mode mode = Mode::Geometric) const;
    Q_INVOKABLE QVariantList dumpOrder(Mode mode = Mode::Geometric) const;
    Q_INVOKABLE bool contains(QQuickItem *item) const;
    Q_INVOKABLE bool focusable(QQuickItem *item) const;

signals:
    void scopeChanged();
    void policyChanged();

private:
    QList<QQuickItem *> collectFocusable() const;
    QList<QQuickItem *> orderedItems(Mode mode) const;
    QPointF centerInScope(QQuickItem *item) const;
    int explicitOrder(QQuickItem *item) const;

    QPointer<QQuickItem> m_scope;
    QMetaObject::Connection m_scopeDestroyedConnection;
    qreal m_rowTolerance = 8.0;
    Qt::LayoutDirection m_layoutDirection = Qt::LeftToRight;
};

} // namespace LfQuick
