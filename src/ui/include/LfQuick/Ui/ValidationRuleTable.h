#pragma once

#include <LfQuick/Core/Validation.h>
#include <LfQuick/Ui/LfQuickUiGlobal.h>

#include <QObject>
#include <QString>
#include <QVariantMap>
#include <QtQmlIntegration/qqmlintegration.h>

#include <memory>

namespace LfQuick {

class LFQUICKUI_EXPORT ValidationRuleTable : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(QVariantMap rules READ rules WRITE setRules NOTIFY rulesChanged FINAL)
    Q_PROPERTY(int count READ count NOTIFY rulesChanged FINAL)
    Q_PROPERTY(bool hasProvider READ hasProvider NOTIFY providerChanged FINAL)

public:
    explicit ValidationRuleTable(QObject *parent = nullptr);

    QVariantMap rules() const { return m_rules; }
    void setRules(const QVariantMap &rules);
    int count() const noexcept { return m_rules.size(); }

    bool hasProvider() const noexcept { return static_cast<bool>(m_provider); }
    void setProvider(std::shared_ptr<IValidationRuleProvider> provider);
    std::shared_ptr<IValidationRuleProvider> provider() const { return m_provider; }

    Q_INVOKABLE bool setRule(const QString &ruleId, const QVariantMap &rule);
    Q_INVOKABLE bool removeRule(const QString &ruleId);
    Q_INVOKABLE void clear();
    Q_INVOKABLE bool contains(const QString &ruleId) const;
    Q_INVOKABLE QVariantMap rule(const QString &ruleId) const;
    Q_INVOKABLE QVariantMap validate(const QString &ruleId, const QVariant &value) const;
    Q_INVOKABLE bool isValid(const QString &ruleId, const QVariant &value) const;

signals:
    void rulesChanged();
    void providerChanged();

private:
    static ValidationRule ruleFromMap(const QVariantMap &map);
    static QVariantMap ruleToMap(const ValidationRule &rule);
    QVariantMap resolvedRule(const QString &ruleId) const;

    QVariantMap m_rules;
    std::shared_ptr<IValidationRuleProvider> m_provider;
};

} // namespace LfQuick
