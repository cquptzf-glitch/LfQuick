#pragma once

#include <LfQuick/Diagnostics/LfQuickDiagnosticsGlobal.h>

#include <QMetaObject>
#include <QObject>
#include <QPointer>
#include <QQuickItem>
#include <QVariantList>
#include <QtQmlIntegration/qqmlintegration.h>

namespace LfQuick {

class LFQUICKDIAGNOSTICS_EXPORT AccessibilityAudit : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(QQuickItem* root READ root WRITE setRoot NOTIFY rootChanged FINAL)
    Q_PROPERTY(QVariantList issues READ issues NOTIFY issuesChanged FINAL)
    Q_PROPERTY(int issueCount READ issueCount NOTIFY issuesChanged FINAL)
    Q_PROPERTY(bool includeDisabled READ includeDisabled WRITE setIncludeDisabled NOTIFY policyChanged FINAL)
    Q_PROPERTY(bool includeInvisible READ includeInvisible WRITE setIncludeInvisible NOTIFY policyChanged FINAL)

public:
    explicit AccessibilityAudit(QObject *parent = nullptr);

    QQuickItem *root() const noexcept { return m_root; }
    void setRoot(QQuickItem *root);

    QVariantList issues() const { return m_issues; }
    int issueCount() const noexcept { return m_issues.size(); }

    bool includeDisabled() const noexcept { return m_includeDisabled; }
    void setIncludeDisabled(bool value);
    bool includeInvisible() const noexcept { return m_includeInvisible; }
    void setIncludeInvisible(bool value);

    Q_INVOKABLE QVariantList scan();
    Q_INVOKABLE void clear();

signals:
    void rootChanged();
    void issuesChanged();
    void policyChanged();

private:
    void scanItem(QQuickItem *item, const QString &path, QVariantList &issues) const;
    QString itemLabel(QQuickItem *item) const;
    static QVariantMap issue(const QString &code,
                             const QString &message,
                             const QString &path,
                             QQuickItem *item,
                             const QString &severity = QStringLiteral("warning"));

    QPointer<QQuickItem> m_root;
    QMetaObject::Connection m_rootDestroyedConnection;
    QVariantList m_issues;
    bool m_includeDisabled = false;
    bool m_includeInvisible = false;
};

} // namespace LfQuick
