#include <LfQuick/Runtime/WindowRouter.h>

#include <LfQuick/Runtime/IWindowInstance.h>
#include <LfQuick/Runtime/IWindowProvider.h>
#include <LfQuick/Runtime/WindowProviderRegistry.h>

#include <QSet>

#include <algorithm>

namespace LfQuick {

namespace {

Error routerError(int code, QString message, QVariantMap details = {})
{
    return Error(QStringLiteral("window-router"), code, std::move(message), std::move(details));
}

} // namespace

struct WindowRouter::Record
{
    WindowDescriptor descriptor;
    WindowOpenRequest request;
    std::unique_ptr<IWindowInstance> instance;
    WindowState state = WindowState::Created;
};

WindowRouter::WindowRouter(WindowProviderRegistry *providers, QObject *parent)
    : QObject(parent), m_providers(providers)
{
}

WindowRouter::~WindowRouter()
{
    const auto records = m_instances.values();
    for (const auto &record : records) {
        if (record && record->instance) {
            record->instance->setCloseRequestHandler({});
            record->instance->close();
        }
    }
}

bool WindowRouter::registerWindow(const WindowDescriptor &descriptor)
{
    if (descriptor.id.trimmed().isEmpty() || descriptor.provider.trimmed().isEmpty())
        return false;
    if (m_descriptors.contains(descriptor.id))
        return false;
    m_descriptors.insert(descriptor.id, descriptor);
    return true;
}

bool WindowRouter::unregisterWindow(QStringView windowId)
{
    const auto id = windowId.toString();
    if (findSingleInstance(id) || !instances(id).isEmpty())
        return false;
    return m_descriptors.remove(id);
}

std::optional<WindowDescriptor> WindowRouter::descriptor(QStringView windowId) const
{
    const auto it = m_descriptors.constFind(windowId.toString());
    if (it == m_descriptors.cend())
        return std::nullopt;
    return it.value();
}

WindowResult WindowRouter::open(const WindowOpenRequest &request)
{
    const auto descriptorIt = m_descriptors.constFind(request.windowId);
    if (descriptorIt == m_descriptors.cend()) {
        auto result = WindowResult::failure(request.windowId,
                                            {},
                                            routerError(1, QStringLiteral("Window is not registered")));
        emit windowOpenFailed(request.windowId, result.error.message());
        return result;
    }

    const WindowDescriptor descriptorValue = descriptorIt.value();

    if (!request.ownerInstanceId.isEmpty() && !m_instances.contains(request.ownerInstanceId)) {
        auto result = WindowResult::failure(request.windowId,
                                            {},
                                            routerError(2, QStringLiteral("Owner window instance was not found"),
                                                        {{QStringLiteral("ownerInstanceId"), request.ownerInstanceId}}));
        emit windowOpenFailed(request.windowId, result.error.message());
        return result;
    }

    if (descriptorValue.openPolicy == WindowOpenPolicy::SingleInstance) {
        if (auto existing = findSingleInstance(request.windowId)) {
            const QString existingId = existing->instance->instanceId();
            if (!request.ownerInstanceId.isEmpty() && wouldFormOwnerCycle(existingId, request.ownerInstanceId)) {
                auto result = WindowResult::failure(request.windowId,
                                                    existingId,
                                                    routerError(13, QStringLiteral("Window ownership cannot form a cycle")));
                emit windowOpenFailed(request.windowId, result.error.message());
                return result;
            }
            if (!existing->instance->updateParameters(request.parameters)) {
                auto result = WindowResult::failure(request.windowId,
                                                    existingId,
                                                    routerError(3, QStringLiteral("Existing window rejected updated parameters")));
                emit windowOpenFailed(request.windowId, result.error.message());
                return result;
            }
            if (!request.restoreState.isEmpty() && !existing->instance->restoreState(request.restoreState)) {
                auto result = WindowResult::failure(request.windowId,
                                                    existingId,
                                                    routerError(4, QStringLiteral("Existing window rejected restore state")));
                emit windowOpenFailed(request.windowId, result.error.message());
                return result;
            }
            if (!request.ownerInstanceId.isEmpty()
                && request.ownerInstanceId != existing->request.ownerInstanceId) {
                auto ownerResult = reparent(existingId, request.ownerInstanceId);
                if (!ownerResult.accepted) {
                    emit windowOpenFailed(request.windowId, ownerResult.error.message());
                    return ownerResult;
                }
            }
            existing->request.parameters = request.parameters;
            existing->request.metadata = request.metadata;
            setActive(existing);
            auto result = WindowResult::success(request.windowId, existingId);
            result.diagnostics.insert(QStringLiteral("reused"), true);
            return result;
        }
    }

    if (!m_providers) {
        auto result = WindowResult::failure(request.windowId,
                                            {},
                                            routerError(5, QStringLiteral("Window provider registry is unavailable")));
        emit windowOpenFailed(request.windowId, result.error.message());
        return result;
    }

    const auto provider = m_providers->find(descriptorValue.provider);
    if (!provider) {
        auto result = WindowResult::failure(request.windowId,
                                            {},
                                            routerError(6,
                                                        QStringLiteral("Window provider is not registered"),
                                                        {{QStringLiteral("provider"), descriptorValue.provider}}));
        emit windowOpenFailed(request.windowId, result.error.message());
        return result;
    }

    if (!provider->canCreate(descriptorValue)) {
        auto result = WindowResult::failure(request.windowId,
                                            {},
                                            routerError(7,
                                                        QStringLiteral("Window provider cannot create descriptor"),
                                                        {{QStringLiteral("provider"), descriptorValue.provider}}));
        emit windowOpenFailed(request.windowId, result.error.message());
        return result;
    }

    auto instance = provider->create(descriptorValue, request);
    if (!instance) {
        const auto providerError = provider->lastErrorInfo();
        QVariantMap diagnostics;
        diagnostics.insert(QStringLiteral("provider"), descriptorValue.provider);
        diagnostics.insert(QStringLiteral("providerFailure"), providerError.toVariantMap());
        const QString message = providerError.message.isEmpty()
            ? QStringLiteral("Window provider failed to create instance")
            : providerError.message;
        auto result = WindowResult::failure(request.windowId,
                                            {},
                                            routerError(8, message, diagnostics),
                                            diagnostics);
        emit windowOpenFailed(request.windowId, result.error.message());
        return result;
    }

    const auto instanceId = instance->instanceId().trimmed();
    if (instanceId.isEmpty() || m_instances.contains(instanceId)) {
        instance->setCloseRequestHandler({});
        instance->close();
        auto result = WindowResult::failure(request.windowId,
                                            instanceId,
                                            routerError(9, QStringLiteral("Window provider returned an invalid or duplicate instance id")));
        emit windowOpenFailed(request.windowId, result.error.message());
        return result;
    }

    if (!request.restoreState.isEmpty() && !instance->restoreState(request.restoreState)) {
        instance->setCloseRequestHandler({});
        instance->close();
        auto result = WindowResult::failure(request.windowId,
                                            instanceId,
                                            routerError(10, QStringLiteral("Window instance rejected restore state")));
        emit windowOpenFailed(request.windowId, result.error.message());
        return result;
    }

    auto record = std::make_shared<Record>();
    record->descriptor = descriptorValue;
    record->request = request;
    record->instance = std::move(instance);
    record->instance->setCloseRequestHandler([this, instanceId] {
        if (m_instances.contains(instanceId))
            close(instanceId);
    });
    m_instances.insert(instanceId, record);

    setActive(record);
    emit windowsChanged();
    emit windowOpened(request.windowId, instanceId);

    return WindowResult::success(request.windowId, instanceId);
}

WindowResult WindowRouter::activate(QStringView instanceId)
{
    auto record = findRecord(instanceId);
    if (!record)
        return WindowResult::failure({}, instanceId.toString(), routerError(11, QStringLiteral("Window instance was not found")));

    setActive(record);
    return WindowResult::success(record->descriptor.id, record->instance->instanceId());
}

WindowResult WindowRouter::reparent(QStringView instanceId, QStringView ownerInstanceId)
{
    auto record = findRecord(instanceId);
    if (!record || !record->instance)
        return WindowResult::failure({}, instanceId.toString(), routerError(11, QStringLiteral("Window instance was not found")));

    const QString id = record->instance->instanceId();
    const QString ownerId = ownerInstanceId.toString();
    if (!ownerId.isEmpty() && !m_instances.contains(ownerId)) {
        return WindowResult::failure(record->descriptor.id,
                                     id,
                                     routerError(2, QStringLiteral("Owner window instance was not found"),
                                                 {{QStringLiteral("ownerInstanceId"), ownerId}}));
    }
    if (wouldFormOwnerCycle(id, ownerId)) {
        return WindowResult::failure(record->descriptor.id,
                                     id,
                                     routerError(13, QStringLiteral("Window ownership cannot form a cycle")));
    }
    if (record->request.ownerInstanceId == ownerId)
        return WindowResult::success(record->descriptor.id, id);

    if (!record->instance->canUpdateOwner(ownerId) || !record->instance->updateOwner(ownerId)) {
        return WindowResult::failure(record->descriptor.id,
                                     id,
                                     routerError(14,
                                                 QStringLiteral("Window instance rejected owner update"),
                                                 {{QStringLiteral("ownerInstanceId"), ownerId}}));
    }

    record->request.ownerInstanceId = ownerId;
    emit windowOwnerChanged(record->descriptor.id, id, ownerId);
    emit windowsChanged();
    return WindowResult::success(record->descriptor.id, id);
}

WindowResult WindowRouter::close(QStringView instanceId)
{
    auto record = findRecord(instanceId);
    if (!record)
        return WindowResult::failure({}, instanceId.toString(), routerError(11, QStringLiteral("Window instance was not found")));

    return closeRecords(closeClosure(record), record);
}

WindowResult WindowRouter::closeAll()
{
    if (m_instances.isEmpty())
        return WindowResult::success({}, {});

    QList<std::shared_ptr<Record>> records = m_instances.values();
    std::sort(records.begin(), records.end(), [](const auto &left, const auto &right) {
        return left->request.ownerInstanceId.isEmpty() && !right->request.ownerInstanceId.isEmpty();
    });

    for (const auto &record : records) {
        QString reason;
        if (record && record->instance && !record->instance->canClose(&reason)) {
            emit windowCloseBlocked(record->descriptor.id, record->instance->instanceId(), reason);
            return WindowResult::failure(record->descriptor.id,
                                         record->instance->instanceId(),
                                         routerError(12,
                                                     reason.isEmpty() ? QStringLiteral("Window close was blocked") : reason));
        }
    }

    const QStringList ids = m_instances.keys();
    for (const auto &id : ids) {
        auto record = m_instances.value(id);
        if (!record)
            continue;
        record->state = WindowState::Closing;
        if (record->instance) {
            record->instance->setCloseRequestHandler({});
            record->instance->close();
        }
        record->state = WindowState::Closed;
        emit windowClosed(record->descriptor.id, id);
    }

    m_instances.clear();
    if (!m_activeInstanceId.isEmpty()) {
        m_activeInstanceId.clear();
        emit activeWindowChanged({});
    }
    emit windowsChanged();
    return WindowResult::success({}, {});
}

std::optional<WindowInfo> WindowRouter::find(QStringView instanceId) const
{
    auto record = findRecord(instanceId);
    if (!record)
        return std::nullopt;

    WindowInfo info;
    info.windowId = record->descriptor.id;
    info.instanceId = record->instance->instanceId();
    info.ownerInstanceId = record->request.ownerInstanceId;
    info.state = record->state;
    info.parameters = record->request.parameters;
    return info;
}

QList<WindowInfo> WindowRouter::instances(QStringView windowId) const
{
    QList<WindowInfo> result;
    const QString requestedId = windowId.toString();
    for (const auto &record : m_instances) {
        if (!record || (!requestedId.isEmpty() && record->descriptor.id != requestedId))
            continue;
        WindowInfo info;
        info.windowId = record->descriptor.id;
        info.instanceId = record->instance->instanceId();
        info.ownerInstanceId = record->request.ownerInstanceId;
        info.state = record->state;
        info.parameters = record->request.parameters;
        result.push_back(std::move(info));
    }
    return result;
}

std::shared_ptr<WindowRouter::Record> WindowRouter::findRecord(QStringView instanceId) const
{
    const auto it = m_instances.constFind(instanceId.toString());
    return it == m_instances.cend() ? nullptr : it.value();
}

std::shared_ptr<WindowRouter::Record> WindowRouter::findSingleInstance(QStringView windowId) const
{
    const QString id = windowId.toString();
    for (const auto &record : m_instances) {
        if (record && record->descriptor.id == id)
            return record;
    }
    return {};
}

QList<std::shared_ptr<WindowRouter::Record>> WindowRouter::closeClosure(const std::shared_ptr<Record> &root) const
{
    QList<std::shared_ptr<Record>> result;
    if (!root || !root->instance)
        return result;

    const QString rootId = root->instance->instanceId();
    for (const auto &record : m_instances) {
        if (!record || record->request.ownerInstanceId != rootId)
            continue;
        if (record->descriptor.ownerClosePolicy == OwnerClosePolicy::CloseWithOwner)
            result.append(closeClosure(record));
    }
    result.append(root);
    return result;
}

WindowResult WindowRouter::closeRecords(const QList<std::shared_ptr<Record>> &records,
                                        const std::shared_ptr<Record> &root)
{
    for (const auto &record : records) {
        QString reason;
        if (record && record->instance && !record->instance->canClose(&reason)) {
            emit windowCloseBlocked(record->descriptor.id, record->instance->instanceId(), reason);
            return WindowResult::failure(root->descriptor.id,
                                         root->instance->instanceId(),
                                         routerError(12,
                                                     reason.isEmpty() ? QStringLiteral("Window close was blocked") : reason,
                                                     {{QStringLiteral("blockedInstanceId"), record->instance->instanceId()}}));
        }
    }

    QSet<QString> closingIds;
    for (const auto &record : records) {
        if (record && record->instance)
            closingIds.insert(record->instance->instanceId());
    }

    QList<std::shared_ptr<Record>> detachedSurvivors;
    for (const auto &record : m_instances) {
        if (!record || !record->instance || closingIds.contains(record->instance->instanceId()))
            continue;
        if (!closingIds.contains(record->request.ownerInstanceId)
            || record->descriptor.ownerClosePolicy != OwnerClosePolicy::Detach) {
            continue;
        }
        if (!record->instance->canUpdateOwner({})) {
            return WindowResult::failure(root->descriptor.id,
                                         root->instance->instanceId(),
                                         routerError(14,
                                                     QStringLiteral("Detached child rejected owner removal"),
                                                     {{QStringLiteral("childInstanceId"), record->instance->instanceId()}}));
        }
        detachedSurvivors.append(record);
    }

    for (const auto &record : detachedSurvivors) {
        if (!record->instance->updateOwner({})) {
            return WindowResult::failure(root->descriptor.id,
                                         root->instance->instanceId(),
                                         routerError(14,
                                                     QStringLiteral("Detached child failed owner removal"),
                                                     {{QStringLiteral("childInstanceId"), record->instance->instanceId()}}));
        }
        record->request.ownerInstanceId.clear();
        emit windowOwnerChanged(record->descriptor.id, record->instance->instanceId(), {});
    }

    for (const auto &record : records) {
        if (!record || !record->instance)
            continue;
        const QString id = record->instance->instanceId();
        record->state = WindowState::Closing;
        clearActiveIf(id);
        record->instance->setCloseRequestHandler({});
        record->instance->close();
        record->state = WindowState::Closed;
        m_instances.remove(id);
        emit windowClosed(record->descriptor.id, id);
    }

    emit windowsChanged();
    return WindowResult::success(root->descriptor.id, root->instance->instanceId());
}

bool WindowRouter::wouldFormOwnerCycle(QStringView instanceId, QStringView ownerInstanceId) const
{
    const QString id = instanceId.toString();
    QString cursor = ownerInstanceId.toString();
    while (!cursor.isEmpty()) {
        if (cursor == id)
            return true;
        const auto ownerRecord = findRecord(cursor);
        if (!ownerRecord)
            break;
        cursor = ownerRecord->request.ownerInstanceId;
    }
    return false;
}

void WindowRouter::setActive(const std::shared_ptr<Record> &record)
{
    if (!record || !record->instance)
        return;

    const QString newId = record->instance->instanceId();
    if (m_activeInstanceId == newId) {
        record->instance->activate();
        record->state = WindowState::Active;
        emit windowActivated(record->descriptor.id, newId);
        return;
    }

    if (auto current = findRecord(m_activeInstanceId)) {
        current->instance->deactivate();
        current->state = WindowState::Inactive;
    }

    m_activeInstanceId = newId;
    record->instance->activate();
    record->state = WindowState::Active;
    emit activeWindowChanged(newId);
    emit windowActivated(record->descriptor.id, newId);
}

void WindowRouter::clearActiveIf(QStringView instanceId)
{
    if (m_activeInstanceId != instanceId)
        return;
    m_activeInstanceId.clear();
    emit activeWindowChanged({});
}

} // namespace LfQuick
