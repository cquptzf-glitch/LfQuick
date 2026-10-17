#pragma once

#include <LfQuick/Ui/LfQuickUiGlobal.h>
#include <LfQuick/Ui/OverlayBackend.h>

#include <QHash>
#include <QList>
#include <QMetaObject>
#include <QObject>
#include <QPointer>
#include <QStringList>
#include <QTimer>
#include <QVariantList>
#include <QVariantMap>
#include <QtQmlIntegration/qqmlintegration.h>

#include <optional>

namespace LfQuick {

class BusyToken;
class OperationHandle;

class LFQUICKUI_EXPORT OverlayCoordinator : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(bool busy READ busy NOTIFY busyChanged FINAL)
    Q_PROPERTY(int busyCount READ busyCount NOTIFY busyChanged FINAL)
    Q_PROPERTY(QString busyMessage READ busyMessage NOTIFY busyChanged FINAL)
    Q_PROPERTY(double busyProgress READ busyProgress NOTIFY busyChanged FINAL)
    Q_PROPERTY(bool busyCancelable READ busyCancelable NOTIFY busyChanged FINAL)
    Q_PROPERTY(QVariantList toasts READ toasts NOTIFY toastsChanged FINAL)
    Q_PROPERTY(int toastCount READ toastCount NOTIFY toastsChanged FINAL)
    Q_PROPERTY(int maxVisibleToasts READ maxVisibleToasts WRITE setMaxVisibleToasts NOTIFY toastPolicyChanged FINAL)
    Q_PROPERTY(QString toastPosition READ toastPosition WRITE setToastPosition NOTIFY toastPolicyChanged FINAL)
    Q_PROPERTY(bool dialogActive READ dialogActive NOTIFY dialogChanged FINAL)
    Q_PROPERTY(QVariantMap activeDialog READ activeDialog NOTIFY dialogChanged FINAL)
    Q_PROPERTY(int pendingDialogCount READ pendingDialogCount NOTIFY dialogChanged FINAL)
    Q_PROPERTY(bool externalBackendActive READ externalBackendActive NOTIFY backendChanged FINAL)

public:
    explicit OverlayCoordinator(QObject *parent = nullptr);

    bool busy() const;
    int busyCount() const;
    QString busyMessage() const;
    double busyProgress() const;
    bool busyCancelable() const;

    QVariantList toasts() const;
    int toastCount() const noexcept { return m_toastOrder.size(); }
    int maxVisibleToasts() const noexcept { return m_maxVisibleToasts; }
    void setMaxVisibleToasts(int value);
    QString toastPosition() const { return m_toastPosition; }
    void setToastPosition(const QString &position);

    bool dialogActive() const noexcept { return m_activeDialog.has_value(); }
    QVariantMap activeDialog() const;
    int pendingDialogCount() const noexcept;

    bool externalBackendActive() const noexcept { return static_cast<bool>(m_backend); }
    IOverlayBackend *backend() const noexcept { return m_backend; }
    void setBackend(IOverlayBackend *backend);

    Q_INVOKABLE QString beginBusy(const QString &message = {}, bool cancelable = false);
    Q_INVOKABLE BusyToken *acquireBusy(const QString &message = {}, bool cancelable = false);
    Q_INVOKABLE bool hasBusyToken(const QString &token) const;
    Q_INVOKABLE bool endBusy(const QString &token);
    Q_INVOKABLE QString bindOperation(QObject *operation);
    Q_INVOKABLE void cancelTopBusy();

    Q_INVOKABLE QString showToast(const QString &message, const QVariantMap &options = {});
    Q_INVOKABLE bool dismissToast(const QString &requestId);
    Q_INVOKABLE void clearToasts();

    Q_INVOKABLE QString requestDialog(const QString &title,
                                      const QString &message,
                                      const QVariantMap &options = {});
    Q_INVOKABLE bool resolveDialog(const QString &requestId, const QString &buttonId);
    Q_INVOKABLE bool cancelDialog(const QString &requestId);
    Q_INVOKABLE void clearDialogs();

signals:
    void busyChanged();
    void busyCancelRequested(const QString &token);
    void toastsChanged();
    void toastPolicyChanged();
    void toastShown(const QString &requestId);
    void toastDismissed(const QString &requestId);
    void dialogChanged();
    void dialogRequested(const QString &requestId);
    void dialogResolved(const QString &requestId,
                        const QString &buttonId,
                        const QVariantMap &context);
    void backendChanged();

private:
    struct Entry {
        QString message;
        bool cancelable = false;
        double progress = 0.0;
        QPointer<OperationHandle> operation;
        QMetaObject::Connection progressConnection;
        QMetaObject::Connection messageConnection;
        QMetaObject::Connection cancelableConnection;
        QMetaObject::Connection stateConnection;
        QMetaObject::Connection destroyedConnection;
    };

    struct ToastEntry {
        ToastRequest request;
        QPointer<QTimer> timer;
    };

    QString topToken() const;
    void removeEntry(const QString &token);

    static QString normalizeSeverity(const QString &severity);
    static QString normalizePosition(const QString &position);
    static QString normalizeButton(const QString &buttonId);
    static QStringList buttonsFromVariant(const QVariant &value);
    static QVariantMap toastToMap(const ToastRequest &request);
    static QVariantMap dialogToMap(const DialogRequest &request);

    QString findToastByDedupKey(const QString &dedupKey) const;
    void restartToastTimer(const QString &requestId);
    void removeToastInternal(const QString &requestId, bool notifyBackend);
    void enforceToastLimit();

    void activateNextDialog();
    void presentActiveDialog();
    QString fallbackCancelButton(const DialogRequest &request) const;

    QHash<QString, Entry> m_entries;
    QStringList m_order;

    QHash<QString, ToastEntry> m_toasts;
    QStringList m_toastOrder;
    int m_maxVisibleToasts = 3;
    QString m_toastPosition = QStringLiteral("top-right");

    std::optional<DialogRequest> m_activeDialog;
    QList<DialogRequest> m_dialogQueue;

    QPointer<IOverlayBackend> m_backend;
    QMetaObject::Connection m_backendToastConnection;
    QMetaObject::Connection m_backendDialogConnection;
    QMetaObject::Connection m_backendDestroyedConnection;
};

} // namespace LfQuick
