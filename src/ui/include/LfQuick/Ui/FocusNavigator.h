#pragma once

#include <LfQuick/Ui/LfQuickUiGlobal.h>

#include <QMetaObject>
#include <QObject>
#include <QPointer>
#include <QStringList>
#include <QtQml/qqmlintegration.h>

class QQuickItem;

namespace LfQuick {

class LFQUICKUI_EXPORT FocusNavigator : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(QQuickItem* scope READ scope WRITE setScope NOTIFY scopeChanged FINAL)
    Q_PROPERTY(QQuickItem* currentFocus READ currentFocus NOTIFY currentFocusChanged FINAL)

public:
    explicit FocusNavigator(QObject *parent = nullptr);

    QQuickItem *scope() const;
    void setScope(QQuickItem *scope);

    QQuickItem *currentFocus() const;

    Q_INVOKABLE bool moveNext();
    Q_INVOKABLE bool movePrevious();
    Q_INVOKABLE QStringList focusChain() const;

signals:
    void scopeChanged();
    void currentFocusChanged();

private:
    bool move(bool forward);
    bool isFocusable(QQuickItem *item) const;
    bool isInScope(QQuickItem *item) const;

    QPointer<QQuickItem> m_scope;
    QMetaObject::Connection m_focusConnection;
};

} // namespace LfQuick
