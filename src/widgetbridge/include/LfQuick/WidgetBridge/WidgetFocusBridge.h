#pragma once

#include <LfQuick/WidgetBridge/LfQuickWidgetBridgeGlobal.h>

#include <QObject>
#include <QPointer>
#include <QQuickItem>
#include <QString>

class QKeyEvent;
class QQuickWidget;
class QWidget;

namespace LfQuick {

enum class FocusTransferPolicy {
    Disabled,
    BoundaryOnly
};

class LFQUICKWIDGETBRIDGE_EXPORT WidgetFocusBridge : public QObject
{
    Q_OBJECT
    Q_PROPERTY(FocusTransferPolicy policy READ policy WRITE setPolicy NOTIFY policyChanged FINAL)
    Q_PROPERTY(bool scannerIsolation READ scannerIsolation WRITE setScannerIsolation NOTIFY scannerIsolationChanged FINAL)
    Q_PROPERTY(bool modalFocusTrap READ modalFocusTrap WRITE setModalFocusTrap NOTIFY modalFocusTrapChanged FINAL)

public:
    explicit WidgetFocusBridge(QQuickWidget *surface, QObject *parent = nullptr);
    ~WidgetFocusBridge() override;

    FocusTransferPolicy policy() const noexcept { return m_policy; }
    void setPolicy(FocusTransferPolicy value);

    bool scannerIsolation() const noexcept { return m_scannerIsolation; }
    void setScannerIsolation(bool value);

    bool modalFocusTrap() const noexcept { return m_modalFocusTrap; }
    void setModalFocusTrap(bool value);

    void setBoundaryWidgets(QWidget *before, QWidget *after);
    void setQuickBoundaryItems(QQuickItem *entry, QQuickItem *exit);

    QWidget *beforeWidget() const { return m_before; }
    QWidget *afterWidget() const { return m_after; }
    QQuickItem *entryItem() const { return m_entryItem; }
    QQuickItem *exitItem() const { return m_exitItem; }

    Q_INVOKABLE void rememberFocus();
    Q_INVOKABLE bool restoreFocus();

signals:
    void policyChanged();
    void scannerIsolationChanged();
    void modalFocusTrapChanged();
    void focusTransferred(const QString &direction);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    bool canTransfer() const;
    bool handleWidgetKey(QWidget *widget, QKeyEvent *event);
    bool handleQuickKey(QKeyEvent *event);
    bool focusQuickItem(QQuickItem *item, Qt::FocusReason reason);
    bool focusWidget(QWidget *widget, Qt::FocusReason reason);

    QPointer<QQuickWidget> m_surface;
    QPointer<QWidget> m_before;
    QPointer<QWidget> m_after;
    QPointer<QQuickItem> m_entryItem;
    QPointer<QQuickItem> m_exitItem;
    QPointer<QWidget> m_lastWidgetFocus;
    QPointer<QQuickItem> m_lastQuickFocus;
    FocusTransferPolicy m_policy = FocusTransferPolicy::BoundaryOnly;
    bool m_scannerIsolation = false;
    bool m_modalFocusTrap = true;
};

} // namespace LfQuick

Q_DECLARE_METATYPE(LfQuick::FocusTransferPolicy)
