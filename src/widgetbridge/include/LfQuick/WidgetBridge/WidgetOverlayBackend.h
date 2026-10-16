#pragma once

#include <LfQuick/Ui/OverlayBackend.h>
#include <LfQuick/WidgetBridge/LfQuickWidgetBridgeGlobal.h>

#include <QAbstractButton>
#include <QFrame>
#include <QHash>
#include <QMessageBox>
#include <QPointer>
#include <QStringList>
#include <QWidget>

namespace LfQuick {

class LFQUICKWIDGETBRIDGE_EXPORT WidgetOverlayBackend final : public IOverlayBackend
{
    Q_OBJECT

public:
    explicit WidgetOverlayBackend(QWidget *parentWidget = nullptr,
                                  QObject *parent = nullptr);
    ~WidgetOverlayBackend() override;

    QWidget *parentWidget() const noexcept { return m_parentWidget; }
    void setParentWidget(QWidget *widget);

    void presentToast(const ToastRequest &request) override;
    void dismissToast(const QString &requestId) override;
    void presentDialog(const DialogRequest &request) override;
    void dismissDialog(const QString &requestId) override;

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    struct ToastWidget {
        ToastRequest request;
        QPointer<QFrame> frame;
    };

    struct DialogWidget {
        QPointer<QMessageBox> box;
        QHash<QAbstractButton *, QString> buttonIds;
        QString fallbackButton;
        QMetaObject::Connection finishedConnection;
    };

    QWidget *effectiveParentWidget() const;
    void layoutToasts();
    static QString buttonText(const QString &buttonId);
    static QMessageBox::ButtonRole buttonRole(const QString &buttonId,
                                               const QString &destructiveButton);

    QPointer<QWidget> m_parentWidget;
    QHash<QString, ToastWidget> m_toasts;
    QStringList m_toastOrder;
    QHash<QString, DialogWidget> m_dialogs;
};

} // namespace LfQuick
