#include <LfQuick/Ui/FocusNavigator.h>

#include <QQuickItem>
#include <QQuickWindow>

namespace LfQuick {

FocusNavigator::FocusNavigator(QObject *parent)
    : QObject(parent)
{
}

QQuickItem *FocusNavigator::scope() const
{
    return m_scope;
}

void FocusNavigator::setScope(QQuickItem *scope)
{
    if (m_scope == scope)
        return;

    QObject::disconnect(m_focusConnection);
    m_scope = scope;

    if (auto *window = m_scope ? m_scope->window() : nullptr) {
        m_focusConnection = connect(window, &QQuickWindow::activeFocusItemChanged,
                                    this, &FocusNavigator::currentFocusChanged);
    }

    emit scopeChanged();
    emit currentFocusChanged();
}

QQuickItem *FocusNavigator::currentFocus() const
{
    if (!m_scope || !m_scope->window())
        return nullptr;

    auto *item = m_scope->window()->activeFocusItem();
    return isInScope(item) ? item : nullptr;
}

bool FocusNavigator::moveNext()
{
    return move(true);
}

bool FocusNavigator::movePrevious()
{
    return move(false);
}

QStringList FocusNavigator::focusChain() const
{
    QStringList result;
    if (!m_scope)
        return result;

    QQuickItem *candidate = m_scope->nextItemInFocusChain(true);
    QQuickItem *start = candidate;
    int guard = 0;

    while (candidate && guard++ < 4096) {
        if (candidate == start && guard > 1)
            break;

        if (isInScope(candidate) && isFocusable(candidate)) {
            const QString name = candidate->objectName();
            result.append(name.isEmpty()
                              ? QString::fromLatin1(candidate->metaObject()->className())
                              : name);
        }

        candidate = candidate->nextItemInFocusChain(true);
        if (!candidate)
            break;
    }

    return result;
}

bool FocusNavigator::move(bool forward)
{
    if (!m_scope || !m_scope->window())
        return false;

    QQuickItem *current = currentFocus();
    QQuickItem *candidate = current
        ? current->nextItemInFocusChain(forward)
        : m_scope->nextItemInFocusChain(forward);

    QQuickItem *start = candidate;
    int guard = 0;

    while (candidate && guard++ < 4096) {
        if (isInScope(candidate) && isFocusable(candidate)) {
            candidate->forceActiveFocus(forward ? Qt::TabFocusReason : Qt::BacktabFocusReason);
            emit currentFocusChanged();
            return true;
        }

        candidate = candidate->nextItemInFocusChain(forward);
        if (!candidate || candidate == start)
            break;
    }

    return false;
}

bool FocusNavigator::isFocusable(QQuickItem *item) const
{
    if (!item || !item->isVisible() || !item->isEnabled())
        return false;

    return item->focusPolicy() != Qt::NoFocus || item->activeFocusOnTab();
}

bool FocusNavigator::isInScope(QQuickItem *item) const
{
    return item && m_scope && item != m_scope && m_scope->isAncestorOf(item);
}

} // namespace LfQuick
