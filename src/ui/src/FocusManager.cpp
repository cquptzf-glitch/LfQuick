#include <LfQuick/Ui/FocusManager.h>

#include <QQuickWindow>

namespace LfQuick {

FocusManager::FocusManager(QObject *parent)
    : QObject(parent)
    , m_walker(this)
{
}

void FocusManager::setScope(QQuickItem *scope)
{
    if (m_scope == scope)
        return;

    QObject::disconnect(m_scopeWindowConnection);
    QObject::disconnect(m_focusConnection);
    QObject::disconnect(m_windowDestroyedConnection);
    m_scopeWindowConnection = {};
    m_focusConnection = {};
    m_windowDestroyedConnection = {};

    m_scope = scope;
    m_walker.setScope(scope);
    if (m_scope) {
        m_scopeWindowConnection = connect(m_scope, &QQuickItem::windowChanged,
                                          this, [this](QQuickWindow *) {
            reconnectWindow();
            emit currentFocusChanged();
        });
    }
    reconnectWindow();
    emit scopeChanged();
    emit currentFocusChanged();
}

QQuickItem *FocusManager::currentFocus() const
{
    if (!m_scope || !m_scope->window())
        return nullptr;
    QQuickItem *item = m_scope->window()->activeFocusItem();
    return m_walker.contains(item) ? item : nullptr;
}

void FocusManager::setMode(FocusWalker::Mode mode)
{
    if (m_mode == mode)
        return;
    m_mode = mode;
    emit policyChanged();
}

void FocusManager::setWrap(bool wrap)
{
    if (m_wrap == wrap)
        return;
    m_wrap = wrap;
    emit policyChanged();
}

bool FocusManager::focus(QQuickItem *item, int reason)
{
    if (!m_walker.focusable(item))
        return false;
    item->forceActiveFocus(static_cast<Qt::FocusReason>(reason));
    emit currentFocusChanged();
    emit focusMoved(item);
    return true;
}

bool FocusManager::moveNext()
{
    return moveLinear(true);
}

bool FocusManager::movePrevious()
{
    return moveLinear(false);
}

bool FocusManager::moveLeft()
{
    return moveDirectional(FocusWalker::Direction::Left);
}

bool FocusManager::moveRight()
{
    return moveDirectional(FocusWalker::Direction::Right);
}

bool FocusManager::moveUp()
{
    return moveDirectional(FocusWalker::Direction::Up);
}

bool FocusManager::moveDown()
{
    return moveDirectional(FocusWalker::Direction::Down);
}

QVariantList FocusManager::dumpOrder() const
{
    QVariantList result = m_walker.dumpOrder(m_mode);
    QQuickItem *active = currentFocus();
    for (QVariant &entry : result) {
        QVariantMap map = entry.toMap();
        QObject *item = map.value(QStringLiteral("item")).value<QObject *>();
        map.insert(QStringLiteral("active"), item && item == active);
        entry = map;
    }
    return result;
}

QStringList FocusManager::focusChain() const
{
    QStringList names;
    const QVariantList order = dumpOrder();
    names.reserve(order.size());
    for (const QVariant &entry : order)
        names.append(entry.toMap().value(QStringLiteral("name")).toString());
    return names;
}

bool FocusManager::moveLinear(bool forward)
{
    QQuickItem *candidate = m_walker.next(currentFocus(), m_mode, forward, m_wrap);
    return focus(candidate, forward ? int(Qt::TabFocusReason) : int(Qt::BacktabFocusReason));
}

bool FocusManager::moveDirectional(FocusWalker::Direction direction)
{
    QQuickItem *candidate = m_walker.directional(currentFocus(), direction, m_mode);
    return focus(candidate, int(Qt::OtherFocusReason));
}

void FocusManager::reconnectWindow()
{
    QObject::disconnect(m_focusConnection);
    QObject::disconnect(m_windowDestroyedConnection);
    m_focusConnection = {};
    m_windowDestroyedConnection = {};

    QQuickWindow *window = m_scope ? m_scope->window() : nullptr;
    if (!window)
        return;

    m_focusConnection = connect(window, &QQuickWindow::activeFocusItemChanged,
                                this, &FocusManager::currentFocusChanged);
    m_windowDestroyedConnection = connect(window, &QObject::destroyed, this, [this] {
        m_focusConnection = {};
        m_windowDestroyedConnection = {};
        emit currentFocusChanged();
    });
}

} // namespace LfQuick
