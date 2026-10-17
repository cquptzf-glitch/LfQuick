#include <LfQuick/Runtime/WindowRouter.h>

#include <LfQuick/Runtime/IWindowBacking.h>
#include <LfQuick/Runtime/IWindowInstance.h>
#include <LfQuick/Runtime/IWindowProvider.h>
#include <LfQuick/Runtime/WindowProviderRegistry.h>

#include <memory>
#include <utility>

namespace LfQuick {

// Keep this definition token-equivalent to WindowRouter.cpp. The private
// record is intentionally implementation-only while both translation units
// provide WindowRouter member functions that operate on it.
struct WindowRouter::Record
{
    WindowDescriptor descriptor;
    WindowOpenRequest request;
    std::unique_ptr<IWindowInstance> instance;
    WindowState state = WindowState::Created;
};

namespace {

Error reloadError(int code, QString message, QVariantMap details = {})
{
    return Error(QStringLiteral("window-router"), code,
                 std::move(message), std::move(details));
}

} // namespace

WindowResult WindowRouter::reloadInstance(QStringView instanceId)
{
    const QString id = instanceId.toString();
    auto record = findRecord(id);
    if (!record || !record->instance) {
        auto result = WindowResult::failure(
            {}, id,
            reloadError(11, QStringLiteral("Window instance was not found")));
        emit windowReloadFailed({}, id, result.error.message());
        return result;
    }

    const QString windowId = record->descriptor.id;
    const QString providerId = record->descriptor.provider;
    const WindowState previousState = record->state;

    const auto fail = [this, &windowId, &id](int code,
                                             const QString &message,
                                             QVariantMap details = {}) {
        auto result = WindowResult::failure(
            windowId, id, reloadError(code, message, std::move(details)));
        emit windowReloadFailed(windowId, id, result.error.message());
        return result;
    };

    if (!m_providers) {
        return fail(5, QStringLiteral("Window provider registry is unavailable"));
    }

    const auto provider = m_providers->find(providerId);
    if (!provider) {
        return fail(6,
                    QStringLiteral("Window provider is not registered"),
                    {{QStringLiteral("provider"), providerId}});
    }

    if (!record->instance->canReleaseBacking()) {
        return fail(15,
                    QStringLiteral("Window instance does not support backing transfer"),
                    {{QStringLiteral("provider"), providerId}});
    }

    const QVariantMap savedState = record->instance->saveState();
    auto oldInstance = std::move(record->instance);
    oldInstance->setCloseRequestHandler({});

    std::unique_ptr<IWindowBacking> backing = oldInstance->releaseBacking();
    if (!backing) {
        record->instance = std::move(oldInstance);
        record->instance->setCloseRequestHandler([this, id] {
            if (m_instances.contains(id))
                close(id);
        });
        return fail(16, QStringLiteral("Window instance failed to release its backing"));
    }

    auto restoreOld = [this, &record, &oldInstance, &id, previousState](
                          std::unique_ptr<IWindowBacking> &token) {
        if (!oldInstance || !token
            || !oldInstance->canAdoptBacking(*token)
            || !oldInstance->adoptBacking(token)) {
            return false;
        }

        record->instance = std::move(oldInstance);
        record->state = previousState;
        record->instance->setCloseRequestHandler([this, id] {
            if (m_instances.contains(id))
                close(id);
        });
        return true;
    };

    auto fatalRollbackFailure = [this, &record, &id, &windowId](
                                    std::unique_ptr<IWindowBacking> &token,
                                    const QString &cause) {
        token.reset();
        record->instance.reset();
        record->state = WindowState::Closed;
        clearActiveIf(id);
        m_instances.remove(id);
        emit windowClosed(windowId, id);
        emit windowsChanged();

        const QString message = cause
            + QStringLiteral("; rollback failed and the logical window was removed");
        auto result = WindowResult::failure(
            windowId, id,
            reloadError(21, message,
                        {{QStringLiteral("rollbackFailed"), true}}));
        emit windowReloadFailed(windowId, id, result.error.message());
        return result;
    };

    const auto rollbackOrFail = [&](std::unique_ptr<IWindowBacking> &token,
                                    int code,
                                    const QString &message,
                                    QVariantMap details = {}) {
        if (restoreOld(token))
            return fail(code, message, std::move(details));
        return fatalRollbackFailure(token, message);
    };

    if (backing->providerId() != providerId) {
        return rollbackOrFail(
            backing, 17,
            QStringLiteral("Released backing belongs to a different provider"),
            {{QStringLiteral("provider"), providerId},
             {QStringLiteral("backingProvider"), backing->providerId()}});
    }

    if (backing->instanceId() != id) {
        return rollbackOrFail(
            backing, 19,
            QStringLiteral("Released backing changed the window instance identity"),
            {{QStringLiteral("expectedInstanceId"), id},
             {QStringLiteral("backingInstanceId"), backing->instanceId()}});
    }

    if (!provider->canAdoptBacking(record->descriptor, *backing)) {
        return rollbackOrFail(
            backing, 17,
            QStringLiteral("Window provider rejected its released backing"),
            {{QStringLiteral("provider"), providerId}});
    }

    auto replacement = provider->createWithBacking(
        record->descriptor, record->request, backing);
    if (!replacement) {
        const WindowProviderError providerError = provider->lastErrorInfo();
        QVariantMap details{
            {QStringLiteral("provider"), providerId},
            {QStringLiteral("providerFailure"), providerError.toVariantMap()}
        };
        const QString message = providerError.message.isEmpty()
            ? QStringLiteral("Window provider failed to recreate the logical instance")
            : providerError.message;
        return rollbackOrFail(backing, 18, message, std::move(details));
    }

    // A successful adoption must consume the token. If it does not, the
    // provider has violated the ownership contract; close the unrelated
    // candidate and safely restore the original instance from the token.
    if (backing) {
        replacement->setCloseRequestHandler({});
        replacement->close();
        replacement.reset();
        return rollbackOrFail(
            backing, 18,
            QStringLiteral("Window provider did not consume the adopted backing"));
    }

    auto recoverReplacementBacking = [&replacement]() {
        std::unique_ptr<IWindowBacking> recovered;
        if (replacement && replacement->canReleaseBacking())
            recovered = replacement->releaseBacking();
        return recovered;
    };

    if (replacement->instanceId().trimmed() != id) {
        std::unique_ptr<IWindowBacking> recovered = recoverReplacementBacking();
        replacement.reset();
        if (recovered) {
            return rollbackOrFail(
                recovered, 19,
                QStringLiteral("Recreated window changed its instance identity"),
                {{QStringLiteral("expectedInstanceId"), id}});
        }
        std::unique_ptr<IWindowBacking> none;
        return fatalRollbackFailure(
            none, QStringLiteral("Recreated window changed its instance identity"));
    }

    if (!record->request.parameters.isEmpty()
        && !replacement->updateParameters(record->request.parameters)) {
        std::unique_ptr<IWindowBacking> recovered = recoverReplacementBacking();
        replacement.reset();
        if (recovered) {
            return rollbackOrFail(
                recovered, 3,
                QStringLiteral("Recreated window rejected its parameters"));
        }
        std::unique_ptr<IWindowBacking> none;
        return fatalRollbackFailure(
            none, QStringLiteral("Recreated window rejected its parameters"));
    }

    if (!savedState.isEmpty() && !replacement->restoreState(savedState)) {
        std::unique_ptr<IWindowBacking> recovered = recoverReplacementBacking();
        replacement.reset();
        if (recovered) {
            return rollbackOrFail(
                recovered, 20,
                QStringLiteral("Recreated window rejected its saved state"));
        }
        std::unique_ptr<IWindowBacking> none;
        return fatalRollbackFailure(
            none, QStringLiteral("Recreated window rejected its saved state"));
    }

    replacement->setCloseRequestHandler([this, id] {
        if (m_instances.contains(id))
            close(id);
    });

    record->instance = std::move(replacement);
    record->state = previousState;
    oldInstance.reset();

    if (previousState == WindowState::Active)
        record->instance->activate();
    else if (previousState == WindowState::Inactive)
        record->instance->deactivate();

    emit windowReloaded(windowId, id);
    emit windowsChanged();

    auto result = WindowResult::success(windowId, id);
    result.diagnostics.insert(QStringLiteral("backingPreserved"), true);
    result.diagnostics.insert(QStringLiteral("provider"), providerId);
    return result;
}

} // namespace LfQuick
