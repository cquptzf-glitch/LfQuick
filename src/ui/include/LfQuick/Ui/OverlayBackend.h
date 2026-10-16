#pragma once

#include <LfQuick/Ui/LfQuickUiGlobal.h>

#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariantMap>

namespace LfQuick {

struct LFQUICKUI_EXPORT ToastRequest
{
    QString id;
    QString message;
    QString severity = QStringLiteral("info");
    int durationMs = 3000;
    QString dedupKey;
    QString position = QStringLiteral("top-right");
    QVariantMap context;
};

struct LFQUICKUI_EXPORT DialogRequest
{
    QString id;
    QString title;
    QString message;
    QStringList buttons;
    QString defaultButton;
    QString destructiveButton;
    QVariantMap context;
};

class LFQUICKUI_EXPORT IOverlayBackend : public QObject
{
    Q_OBJECT

public:
    explicit IOverlayBackend(QObject *parent = nullptr)
        : QObject(parent)
    {
    }
    ~IOverlayBackend() override = default;

    virtual void presentToast(const ToastRequest &request) = 0;
    virtual void dismissToast(const QString &requestId) = 0;
    virtual void presentDialog(const DialogRequest &request) = 0;
    virtual void dismissDialog(const QString &requestId) = 0;

signals:
    void toastDismissRequested(const QString &requestId);
    void dialogResolved(const QString &requestId, const QString &buttonId);
};

} // namespace LfQuick
