#include <LfQuick/Diagnostics/AccessibilityAudit.h>

#include <QAccessible>
#include <QAccessibleInterface>

namespace LfQuick {

AccessibilityAudit::AccessibilityAudit(QObject *parent)
    : QObject(parent)
{
}

void AccessibilityAudit::setRoot(QQuickItem *root)
{
    if (m_root == root)
        return;

    QObject::disconnect(m_rootDestroyedConnection);
    m_rootDestroyedConnection = {};
    m_root = root;

    if (m_root) {
        m_rootDestroyedConnection = connect(m_root, &QObject::destroyed, this, [this] {
            m_root = nullptr;
            m_rootDestroyedConnection = {};
            const bool hadIssues = !m_issues.isEmpty();
            m_issues.clear();
            emit rootChanged();
            if (hadIssues)
                emit issuesChanged();
        });
    }

    const bool hadIssues = !m_issues.isEmpty();
    m_issues.clear();
    emit rootChanged();
    if (hadIssues)
        emit issuesChanged();
}

void AccessibilityAudit::setIncludeDisabled(bool value)
{
    if (m_includeDisabled == value)
        return;
    m_includeDisabled = value;
    emit policyChanged();
}

void AccessibilityAudit::setIncludeInvisible(bool value)
{
    if (m_includeInvisible == value)
        return;
    m_includeInvisible = value;
    emit policyChanged();
}

QVariantList AccessibilityAudit::scan()
{
    QVariantList next;
    if (m_root)
        scanItem(m_root, itemLabel(m_root), next);
    m_issues = next;
    emit issuesChanged();
    return m_issues;
}

void AccessibilityAudit::clear()
{
    if (m_issues.isEmpty())
        return;
    m_issues.clear();
    emit issuesChanged();
}

void AccessibilityAudit::scanItem(QQuickItem *item,
                                  const QString &path,
                                  QVariantList &issues) const
{
    if (!item)
        return;

    const bool visible = item->isVisible();
    const bool enabled = item->isEnabled();
    const bool focusable = item->focusPolicy() != Qt::NoFocus || item->activeFocusOnTab();
    const bool shouldAudit = focusable
        && (m_includeInvisible || visible)
        && (m_includeDisabled || enabled);

    if (shouldAudit) {
        QAccessibleInterface *iface = QAccessible::queryAccessibleInterface(item);
        if (!iface || !iface->isValid()) {
            issues.push_back(issue(QStringLiteral("missing-interface"),
                                   QStringLiteral("Focusable item has no valid accessible interface"),
                                   path, item));
        } else {
            const QString name = iface->text(QAccessible::Name).trimmed();
            if (name.isEmpty()) {
                issues.push_back(issue(QStringLiteral("missing-name"),
                                       QStringLiteral("Focusable item has no accessible name"),
                                       path, item));
            }
            if (iface->role() == QAccessible::NoRole) {
                issues.push_back(issue(QStringLiteral("missing-role"),
                                       QStringLiteral("Focusable item exposes no accessible role"),
                                       path, item));
            }
        }
    }

    for (QQuickItem *child : item->childItems()) {
        const QString childPath = path + QLatin1Char('/') + itemLabel(child);
        scanItem(child, childPath, issues);
    }
}

QString AccessibilityAudit::itemLabel(QQuickItem *item) const
{
    if (!item)
        return QStringLiteral("<null>");
    if (!item->objectName().isEmpty())
        return item->objectName();
    return QString::fromLatin1(item->metaObject()->className());
}

QVariantMap AccessibilityAudit::issue(const QString &code,
                                      const QString &message,
                                      const QString &path,
                                      QQuickItem *item,
                                      const QString &severity)
{
    // Keep audit snapshots stable after the UI tree mutates. Raw QObject/QQuickItem
    // pointers would become dangling if a control is destroyed before the next scan.
    return {
        {QStringLiteral("code"), code},
        {QStringLiteral("message"), message},
        {QStringLiteral("severity"), severity},
        {QStringLiteral("path"), path},
        {QStringLiteral("objectName"), item ? item->objectName() : QString{}},
        {QStringLiteral("className"), item ? QString::fromLatin1(item->metaObject()->className()) : QString{}}
    };
}

} // namespace LfQuick
