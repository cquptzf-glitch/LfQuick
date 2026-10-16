#pragma once

#include <LfQuick/Core/Permission.h>
#include <LfQuick/Ui/LfQuickUiGlobal.h>

#include <QObject>
#include <QString>
#include <QVariantMap>
#include <QtQmlIntegration/qqmlintegration.h>

#include <memory>

namespace LfQuick {

class LFQUICKUI_EXPORT PermissionEvaluator : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(bool hasProvider READ hasProvider NOTIFY providerChanged FINAL)

public:
    enum class State {
        Allow = static_cast<int>(PermissionState::Allow),
        ReadOnly = static_cast<int>(PermissionState::ReadOnly),
        Disabled = static_cast<int>(PermissionState::Disabled),
        Hidden = static_cast<int>(PermissionState::Hidden),
        Denied = static_cast<int>(PermissionState::Denied)
    };
    Q_ENUM(State)

    explicit PermissionEvaluator(QObject *parent = nullptr);

    bool hasProvider() const noexcept { return static_cast<bool>(m_provider); }
    void setProvider(std::shared_ptr<IPermissionProvider> provider);
    std::shared_ptr<IPermissionProvider> provider() const { return m_provider; }

    Q_INVOKABLE State evaluate(const QString &permissionId,
                               const QVariantMap &context = {}) const;
    Q_INVOKABLE bool allows(const QString &permissionId,
                            const QVariantMap &context = {}) const;
    Q_INVOKABLE bool isVisible(const QString &permissionId,
                               const QVariantMap &context = {}) const;
    Q_INVOKABLE bool isEnabled(const QString &permissionId,
                               const QVariantMap &context = {}) const;
    Q_INVOKABLE bool isEditable(const QString &permissionId,
                                const QVariantMap &context = {}) const;

signals:
    void providerChanged();

private:
    std::shared_ptr<IPermissionProvider> m_provider;
};

} // namespace LfQuick
