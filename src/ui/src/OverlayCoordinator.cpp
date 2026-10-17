#include <LfQuick/Ui/OverlayCoordinator.h>

#include <LfQuick/Core/OperationHandle.h>

#include <QSet>
#include <QTimer>
#include <QUuid>

#include <utility>

namespace LfQuick {

OverlayCoordinator::OverlayCoordinator(QObject *parent)
    : QObject(parent)
{
}

bool OverlayCoordinator::busy() const
{
    return !m_order.isEmpty();
}

int OverlayCoordinator::busyCount() const
{
    return m_order.size();
}

QString OverlayCoordinator::busyMessage() const
{
    const QString token = topToken();
    return token.isEmpty() ? QString{} : m_entries.value(token).message;
}

double OverlayCoordinator::busyProgress() const
{
    const QString token = topToken();
    return token.isEmpty() ? 0.0 : m_entries.value(token).progress;
}

bool OverlayCoordinator::busyCancelable() const
{
    const QString token = topToken();
    return !token.isEmpty() && m_entries.value(token).cancelable;
}

QVariantList OverlayCoordinator::toasts() const
{
    QVariantList result;
    result.reserve(m_toastOrder.size());
    for (const QString &id : m_toastOrder) {
        const auto it = m_toasts.constFind(id);
        if (it != m_toasts.cend())
            result.push_back(toastToMap(it->request));
    }
    return result;
}

void OverlayCoordinator::setMaxVisibleToasts(int value)
{
    value = qBound(1, value, 10);
    if (m_maxVisibleToasts == value)
        return;

    m_maxVisibleToasts = value;
    enforceToastLimit();
    emit toastPolicyChanged();
    emit toastsChanged();
}

void OverlayCoordinator::setToastPosition(const QString &position)
{
    const QString normalized = normalizePosition(position);
    if (m_toastPosition == normalized)
        return;

    m_toastPosition = normalized;
    for (const QString &id : m_toastOrder) {
        auto it = m_toasts.find(id);
        if (it == m_toasts.end())
            continue;
        it->request.position = normalized;
        if (m_backend) {
            m_backend->dismissToast(id);
            m_backend->presentToast(it->request);
        }
    }
    emit toastPolicyChanged();
    emit toastsChanged();
}

QVariantMap OverlayCoordinator::activeDialog() const
{
    return m_activeDialog ? dialogToMap(*m_activeDialog) : QVariantMap{};
}

int OverlayCoordinator::pendingDialogCount() const noexcept
{
    return m_dialogQueue.size() + (m_activeDialog ? 1 : 0);
}

void OverlayCoordinator::setBackend(IOverlayBackend *backend)
{
    if (m_backend == backend)
        return;

    if (m_backend) {
        for (const QString &id : m_toastOrder)
            m_backend->dismissToast(id);
        if (m_activeDialog)
            m_backend->dismissDialog(m_activeDialog->id);
    }

    QObject::disconnect(m_backendToastConnection);
    QObject::disconnect(m_backendDialogConnection);
    QObject::disconnect(m_backendDestroyedConnection);
    m_backendToastConnection = {};
    m_backendDialogConnection = {};
    m_backendDestroyedConnection = {};
    m_backend = backend;

    if (m_backend) {
        m_backendToastConnection = connect(
            m_backend, &IOverlayBackend::toastDismissRequested,
            this, [this](const QString &requestId) { dismissToast(requestId); });
        m_backendDialogConnection = connect(
            m_backend, &IOverlayBackend::dialogResolved,
            this, [this](const QString &requestId, const QString &buttonId) {
                resolveDialog(requestId, buttonId);
            });
        m_backendDestroyedConnection = connect(
            m_backend, &QObject::destroyed, this, [this] {
                m_backend = nullptr;
                m_backendToastConnection = {};
                m_backendDialogConnection = {};
                m_backendDestroyedConnection = {};
                emit backendChanged();
            });

        for (const QString &id : m_toastOrder) {
            const auto it = m_toasts.constFind(id);
            if (it != m_toasts.cend())
                m_backend->presentToast(it->request);
        }
        presentActiveDialog();
    }

    emit backendChanged();
}

QString OverlayCoordinator::beginBusy(const QString &message, bool cancelable)
{
    const QString token = QUuid::createUuid().toString(QUuid::WithoutBraces);

    Entry entry;
    entry.message = message;
    entry.cancelable = cancelable;
    m_entries.insert(token, entry);
    m_order.append(token);

    emit busyChanged();
    return token;
}

bool OverlayCoordinator::endBusy(const QString &token)
{
    if (token.isEmpty() || !m_entries.contains(token))
        return false;

    removeEntry(token);
    emit busyChanged();
    return true;
}

QString OverlayCoordinator::bindOperation(QObject *object)
{
    auto *operation = qobject_cast<OperationHandle *>(object);
    if (!operation)
        return {};

    const auto initialState = operation->state();
    if (initialState == OperationHandle::State::Succeeded
        || initialState == OperationHandle::State::Failed
        || initialState == OperationHandle::State::Canceled) {
        return {};
    }

    const QString token = beginBusy(operation->message(), operation->cancelable());
    auto it = m_entries.find(token);
    it->operation = operation;
    it->progress = operation->progress();

    const QPointer<OperationHandle> guardedOperation = operation;

    it->progressConnection = connect(operation, &OperationHandle::progressChanged, this,
                                     [this, token, guardedOperation] {
        if (!guardedOperation || !m_entries.contains(token))
            return;
        m_entries[token].progress = guardedOperation->progress();
        emit busyChanged();
    });

    it->messageConnection = connect(operation, &OperationHandle::messageChanged, this,
                                    [this, token, guardedOperation] {
        if (!guardedOperation || !m_entries.contains(token))
            return;
        m_entries[token].message = guardedOperation->message();
        emit busyChanged();
    });

    it->cancelableConnection = connect(operation, &OperationHandle::cancelableChanged, this,
                                       [this, token, guardedOperation] {
        if (!guardedOperation || !m_entries.contains(token))
            return;
        m_entries[token].cancelable = guardedOperation->cancelable();
        emit busyChanged();
    });

    it->stateConnection = connect(operation, &OperationHandle::stateChanged, this,
                                   [this, token, guardedOperation] {
        if (!guardedOperation || !m_entries.contains(token))
            return;

        const auto state = guardedOperation->state();
        if (state == OperationHandle::State::Succeeded
            || state == OperationHandle::State::Failed
            || state == OperationHandle::State::Canceled) {
            endBusy(token);
        } else {
            emit busyChanged();
        }
    });

    it->destroyedConnection = connect(operation, &QObject::destroyed, this,
                                      [this, token] {
        if (m_entries.contains(token))
            endBusy(token);
    });

    emit busyChanged();
    return token;
}

void OverlayCoordinator::cancelTopBusy()
{
    const QString token = topToken();
    if (token.isEmpty())
        return;

    const auto it = m_entries.constFind(token);
    if (it == m_entries.cend())
        return;

    if (it->operation && it->cancelable) {
        it->operation->cancel();
        return;
    }

    if (it->cancelable)
        emit busyCancelRequested(token);
}

QString OverlayCoordinator::showToast(const QString &message, const QVariantMap &options)
{
    const QString normalizedMessage = message.trimmed();
    if (normalizedMessage.isEmpty())
        return {};

    bool durationOk = false;
    int durationMs = options.value(QStringLiteral("durationMs"), 3000).toInt(&durationOk);
    if (!durationOk)
        durationMs = 3000;
    durationMs = qBound(0, durationMs, 600000);

    ToastRequest request;
    request.message = normalizedMessage;
    request.severity = normalizeSeverity(options.value(QStringLiteral("severity"),
                                                        QStringLiteral("info")).toString());
    request.durationMs = durationMs;
    request.position = normalizePosition(options.value(QStringLiteral("position"),
                                                        m_toastPosition).toString());
    request.context = options.value(QStringLiteral("context")).toMap();
    request.dedupKey = options.value(QStringLiteral("dedupKey")).toString().trimmed();
    if (request.dedupKey.isEmpty())
        request.dedupKey = request.severity + QLatin1Char('|') + request.message;

    const QString existingId = findToastByDedupKey(request.dedupKey);
    if (!existingId.isEmpty()) {
        auto it = m_toasts.find(existingId);
        if (it == m_toasts.end())
            return {};
        request.id = existingId;
        it->request = request;
        m_toastOrder.removeAll(existingId);
        m_toastOrder.append(existingId);
        restartToastTimer(existingId);
        if (m_backend) {
            m_backend->dismissToast(existingId);
            m_backend->presentToast(request);
        }
        emit toastsChanged();
        return existingId;
    }

    request.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    ToastEntry entry;
    entry.request = request;
    m_toasts.insert(request.id, entry);
    m_toastOrder.append(request.id);
    restartToastTimer(request.id);

    if (m_backend)
        m_backend->presentToast(request);

    enforceToastLimit();
    emit toastsChanged();
    emit toastShown(request.id);
    return request.id;
}

bool OverlayCoordinator::dismissToast(const QString &requestId)
{
    const QString id = requestId.trimmed();
    if (id.isEmpty() || !m_toasts.contains(id))
        return false;

    removeToastInternal(id, true);
    emit toastsChanged();
    emit toastDismissed(id);
    return true;
}

void OverlayCoordinator::clearToasts()
{
    if (m_toastOrder.isEmpty())
        return;

    const QStringList ids = m_toastOrder;
    for (const QString &id : ids) {
        removeToastInternal(id, true);
        emit toastDismissed(id);
    }
    emit toastsChanged();
}

QString OverlayCoordinator::requestDialog(const QString &title,
                                          const QString &message,
                                          const QVariantMap &options)
{
    const QString normalizedTitle = title.trimmed();
    const QString normalizedMessage = message.trimmed();
    if (normalizedTitle.isEmpty() && normalizedMessage.isEmpty())
        return {};

    static const QSet<QString> allowedButtons{
        QStringLiteral("ok"), QStringLiteral("cancel"), QStringLiteral("yes"),
        QStringLiteral("no"), QStringLiteral("retry"), QStringLiteral("close"),
        QStringLiteral("discard"), QStringLiteral("apply")
    };

    QStringList buttons;
    for (const QString &rawButton : buttonsFromVariant(options.value(QStringLiteral("buttons")))) {
        const QString button = normalizeButton(rawButton);
        if (!button.isEmpty() && allowedButtons.contains(button) && !buttons.contains(button))
            buttons.append(button);
    }
    if (buttons.isEmpty())
        buttons.append(QStringLiteral("ok"));

    QString defaultButton = normalizeButton(
        options.value(QStringLiteral("defaultButton")).toString());
    if (!buttons.contains(defaultButton))
        defaultButton = buttons.constFirst();

    QString destructiveButton = normalizeButton(
        options.value(QStringLiteral("destructiveButton")).toString());
    if (!buttons.contains(destructiveButton))
        destructiveButton.clear();

    DialogRequest request;
    request.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    request.title = normalizedTitle;
    request.message = normalizedMessage;
    request.buttons = buttons;
    request.defaultButton = defaultButton;
    request.destructiveButton = destructiveButton;
    request.context = options.value(QStringLiteral("context")).toMap();

    if (!m_activeDialog) {
        m_activeDialog = request;
        presentActiveDialog();
        emit dialogRequested(request.id);
    } else {
        m_dialogQueue.append(request);
    }
    emit dialogChanged();
    return request.id;
}

bool OverlayCoordinator::resolveDialog(const QString &requestId, const QString &buttonId)
{
    if (!m_activeDialog || m_activeDialog->id != requestId.trimmed())
        return false;

    const QString normalizedButton = normalizeButton(buttonId);
    if (!m_activeDialog->buttons.contains(normalizedButton))
        return false;

    const DialogRequest resolved = *m_activeDialog;
    if (m_backend)
        m_backend->dismissDialog(resolved.id);
    m_activeDialog.reset();

    emit dialogResolved(resolved.id, normalizedButton, resolved.context);
    if (!m_dialogQueue.isEmpty())
        activateNextDialog();
    else
        emit dialogChanged();
    return true;
}

bool OverlayCoordinator::cancelDialog(const QString &requestId)
{
    const QString id = requestId.trimmed();
    if (id.isEmpty())
        return false;

    if (m_activeDialog && m_activeDialog->id == id)
        return resolveDialog(id, fallbackCancelButton(*m_activeDialog));

    for (int i = 0; i < m_dialogQueue.size(); ++i) {
        if (m_dialogQueue.at(i).id != id)
            continue;
        const DialogRequest request = m_dialogQueue.takeAt(i);
        emit dialogResolved(request.id, fallbackCancelButton(request), request.context);
        emit dialogChanged();
        return true;
    }
    return false;
}

void OverlayCoordinator::clearDialogs()
{
    if (!m_activeDialog && m_dialogQueue.isEmpty())
        return;

    QList<DialogRequest> requests;
    if (m_activeDialog) {
        if (m_backend)
            m_backend->dismissDialog(m_activeDialog->id);
        requests.append(*m_activeDialog);
    }
    requests.append(m_dialogQueue);
    m_activeDialog.reset();
    m_dialogQueue.clear();

    for (const DialogRequest &request : std::as_const(requests))
        emit dialogResolved(request.id, fallbackCancelButton(request), request.context);
    emit dialogChanged();
}

QString OverlayCoordinator::topToken() const
{
    return m_order.isEmpty() ? QString{} : m_order.constLast();
}

void OverlayCoordinator::removeEntry(const QString &token)
{
    auto it = m_entries.find(token);
    if (it == m_entries.end())
        return;

    QObject::disconnect(it->progressConnection);
    QObject::disconnect(it->messageConnection);
    QObject::disconnect(it->cancelableConnection);
    QObject::disconnect(it->stateConnection);
    QObject::disconnect(it->destroyedConnection);

    m_entries.erase(it);
    m_order.removeAll(token);
}

QString OverlayCoordinator::normalizeSeverity(const QString &severity)
{
    const QString normalized = severity.trimmed().toLower();
    if (normalized == QStringLiteral("success")
        || normalized == QStringLiteral("warning")
        || normalized == QStringLiteral("error")) {
        return normalized;
    }
    return QStringLiteral("info");
}

QString OverlayCoordinator::normalizePosition(const QString &position)
{
    const QString normalized = position.trimmed().toLower();
    if (normalized == QStringLiteral("top-center")
        || normalized == QStringLiteral("bottom-right")
        || normalized == QStringLiteral("bottom-center")) {
        return normalized;
    }
    return QStringLiteral("top-right");
}

QString OverlayCoordinator::normalizeButton(const QString &buttonId)
{
    return buttonId.trimmed().toLower();
}

QStringList OverlayCoordinator::buttonsFromVariant(const QVariant &value)
{
    QStringList result;
    if (value.canConvert<QStringList>())
        result = value.toStringList();
    if (!result.isEmpty())
        return result;

    const QVariantList list = value.toList();
    result.reserve(list.size());
    for (const QVariant &entry : list)
        result.append(entry.toString());
    return result;
}

QVariantMap OverlayCoordinator::toastToMap(const ToastRequest &request)
{
    return {
        {QStringLiteral("id"), request.id},
        {QStringLiteral("message"), request.message},
        {QStringLiteral("severity"), request.severity},
        {QStringLiteral("durationMs"), request.durationMs},
        {QStringLiteral("dedupKey"), request.dedupKey},
        {QStringLiteral("position"), request.position},
        {QStringLiteral("context"), request.context}
    };
}

QVariantMap OverlayCoordinator::dialogToMap(const DialogRequest &request)
{
    return {
        {QStringLiteral("id"), request.id},
        {QStringLiteral("title"), request.title},
        {QStringLiteral("message"), request.message},
        {QStringLiteral("buttons"), request.buttons},
        {QStringLiteral("defaultButton"), request.defaultButton},
        {QStringLiteral("destructiveButton"), request.destructiveButton},
        {QStringLiteral("context"), request.context}
    };
}

QString OverlayCoordinator::findToastByDedupKey(const QString &dedupKey) const
{
    for (const QString &id : m_toastOrder) {
        const auto it = m_toasts.constFind(id);
        if (it != m_toasts.cend() && it->request.dedupKey == dedupKey)
            return id;
    }
    return {};
}

void OverlayCoordinator::restartToastTimer(const QString &requestId)
{
    auto it = m_toasts.find(requestId);
    if (it == m_toasts.end())
        return;

    if (it->timer) {
        it->timer->stop();
        it->timer->deleteLater();
        it->timer = nullptr;
    }

    if (it->request.durationMs <= 0)
        return;

    auto *timer = new QTimer(this);
    timer->setSingleShot(true);
    timer->setInterval(it->request.durationMs);
    connect(timer, &QTimer::timeout, this, [this, requestId] {
        dismissToast(requestId);
    });
    it->timer = timer;
    timer->start();
}

void OverlayCoordinator::removeToastInternal(const QString &requestId, bool notifyBackend)
{
    auto it = m_toasts.find(requestId);
    if (it == m_toasts.end())
        return;

    if (notifyBackend && m_backend)
        m_backend->dismissToast(requestId);
    if (it->timer) {
        it->timer->stop();
        it->timer->deleteLater();
    }
    m_toasts.erase(it);
    m_toastOrder.removeAll(requestId);
}

void OverlayCoordinator::enforceToastLimit()
{
    while (m_toastOrder.size() > m_maxVisibleToasts) {
        const QString id = m_toastOrder.constFirst();
        removeToastInternal(id, true);
        emit toastDismissed(id);
    }
}

void OverlayCoordinator::activateNextDialog()
{
    if (m_activeDialog || m_dialogQueue.isEmpty())
        return;
    m_activeDialog = m_dialogQueue.takeFirst();
    presentActiveDialog();
    emit dialogRequested(m_activeDialog->id);
    emit dialogChanged();
}

void OverlayCoordinator::presentActiveDialog()
{
    if (m_backend && m_activeDialog)
        m_backend->presentDialog(*m_activeDialog);
}

QString OverlayCoordinator::fallbackCancelButton(const DialogRequest &request) const
{
    if (request.buttons.contains(QStringLiteral("cancel")))
        return QStringLiteral("cancel");
    if (request.buttons.contains(QStringLiteral("close")))
        return QStringLiteral("close");
    if (request.buttons.contains(QStringLiteral("no")))
        return QStringLiteral("no");
    if (request.buttons.contains(request.defaultButton))
        return request.defaultButton;
    return request.buttons.isEmpty() ? QStringLiteral("ok") : request.buttons.constFirst();
}

} // namespace LfQuick