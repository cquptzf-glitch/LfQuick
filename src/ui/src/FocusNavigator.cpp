#include <LfQuick/Ui/FocusNavigator.h>
#include <LfQuick/Ui/LfAttached.h>

#include <QQuickItem>
#include <QQuickWindow>
#include <QtQml/qqml.h>

#include <algorithm>
#include <cmath>
#include <limits>

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

    QObject::disconnect(m_scopeDestroyedConnection);
    QObject::disconnect(m_scopeWindowConnection);
    QObject::disconnect(m_focusConnection);
    QObject::disconnect(m_windowDestroyedConnection);
    m_scopeDestroyedConnection = {};
    m_scopeWindowConnection = {};
    m_focusConnection = {};
    m_windowDestroyedConnection = {};

    m_scope = scope;
    if (m_scope) {
        m_scopeDestroyedConnection = connect(m_scope, &QObject::destroyed, this, [this] {
            m_scope = nullptr;
            m_scopeDestroyedConnection = {};
            m_scopeWindowConnection = {};
            QObject::disconnect(m_focusConnection);
            QObject::disconnect(m_windowDestroyedConnection);
            m_focusConnection = {};
            m_windowDestroyedConnection = {};
            emit scopeChanged();
            emit currentFocusChanged();
        });
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

QQuickItem *FocusNavigator::currentFocus() const
{
    if (!m_scope || !m_scope->window())
        return nullptr;

    auto *item = m_scope->window()->activeFocusItem();
    return isInScope(item) ? item : nullptr;
}

void FocusNavigator::setMode(Mode mode)
{
    if (m_mode == mode)
        return;
    m_mode = mode;
    emit policyChanged();
}

void FocusNavigator::setWrap(bool wrap)
{
    if (m_wrap == wrap)
        return;
    m_wrap = wrap;
    emit policyChanged();
}

void FocusNavigator::setRowTolerance(qreal tolerance)
{
    tolerance = std::max<qreal>(0.0, tolerance);
    if (qFuzzyCompare(m_rowTolerance, tolerance))
        return;
    m_rowTolerance = tolerance;
    emit policyChanged();
}

bool FocusNavigator::moveNext()
{
    return moveLinear(true);
}

bool FocusNavigator::movePrevious()
{
    return moveLinear(false);
}

bool FocusNavigator::moveLeft()
{
    return moveDirectional(Direction::Left);
}

bool FocusNavigator::moveRight()
{
    return moveDirectional(Direction::Right);
}

bool FocusNavigator::moveUp()
{
    return moveDirectional(Direction::Up);
}

bool FocusNavigator::moveDown()
{
    return moveDirectional(Direction::Down);
}

QStringList FocusNavigator::focusChain() const
{
    QStringList result;
    const auto items = orderedItems();
    result.reserve(items.size());
    for (QQuickItem *item : items) {
        const QString name = item->objectName();
        result.append(name.isEmpty()
                          ? QString::fromLatin1(item->metaObject()->className())
                          : name);
    }
    return result;
}

QVariantList FocusNavigator::dumpOrder() const
{
    QVariantList result;
    const auto items = orderedItems();
    result.reserve(items.size());
    for (int index = 0; index < items.size(); ++index) {
        QQuickItem *item = items.at(index);
        const QPointF center = itemCenter(item);
        result.push_back(QVariantMap{
            {QStringLiteral("index"), index},
            {QStringLiteral("name"), item->objectName().isEmpty()
                ? QString::fromLatin1(item->metaObject()->className())
                : item->objectName()},
            {QStringLiteral("focusOrder"), explicitOrder(item)},
            {QStringLiteral("x"), center.x()},
            {QStringLiteral("y"), center.y()},
            {QStringLiteral("width"), item->width()},
            {QStringLiteral("height"), item->height()},
            {QStringLiteral("active"), item == currentFocus()}
        });
    }
    return result;
}

bool FocusNavigator::moveLinear(bool forward)
{
    if (!m_scope || !m_scope->window())
        return false;

    if (m_mode == Mode::Native) {
        QQuickItem *current = currentFocus();
        QQuickItem *candidate = current
            ? current->nextItemInFocusChain(forward)
            : m_scope->nextItemInFocusChain(forward);
        QQuickItem *start = candidate;
        int guard = 0;

        while (candidate && guard++ < 4096) {
            if (isInScope(candidate) && isFocusable(candidate))
                return focusItem(candidate, forward ? Qt::TabFocusReason : Qt::BacktabFocusReason);

            candidate = candidate->nextItemInFocusChain(forward);
            if (!candidate || candidate == start)
                break;
        }
        return false;
    }

    const auto items = orderedItems();
    if (items.isEmpty())
        return false;

    QQuickItem *current = currentFocus();
    int currentIndex = items.indexOf(current);
    if (currentIndex < 0)
        return focusItem(forward ? items.first() : items.last(),
                         forward ? Qt::TabFocusReason : Qt::BacktabFocusReason);

    int nextIndex = currentIndex + (forward ? 1 : -1);
    if (nextIndex < 0 || nextIndex >= items.size()) {
        if (!m_wrap)
            return false;
        nextIndex = forward ? 0 : items.size() - 1;
    }

    return focusItem(items.at(nextIndex),
                     forward ? Qt::TabFocusReason : Qt::BacktabFocusReason);
}

bool FocusNavigator::moveDirectional(Direction direction)
{
    if (!m_scope || !m_scope->window())
        return false;

    const auto items = focusableItems();
    if (items.isEmpty())
        return false;

    QQuickItem *current = currentFocus();
    if (!current)
        return focusItem(orderedItems().value(0), Qt::OtherFocusReason);

    const QPointF origin = itemCenter(current);
    QQuickItem *best = nullptr;
    qreal bestScore = std::numeric_limits<qreal>::max();

    for (QQuickItem *candidate : items) {
        if (candidate == current)
            continue;

        const QPointF center = itemCenter(candidate);
        const qreal dx = center.x() - origin.x();
        const qreal dy = center.y() - origin.y();

        qreal primary = 0.0;
        qreal secondary = 0.0;
        bool valid = false;
        switch (direction) {
        case Direction::Left:
            valid = dx < 0;
            primary = -dx;
            secondary = std::abs(dy);
            break;
        case Direction::Right:
            valid = dx > 0;
            primary = dx;
            secondary = std::abs(dy);
            break;
        case Direction::Up:
            valid = dy < 0;
            primary = -dy;
            secondary = std::abs(dx);
            break;
        case Direction::Down:
            valid = dy > 0;
            primary = dy;
            secondary = std::abs(dx);
            break;
        }

        if (!valid)
            continue;

        const qreal secondaryWeight = m_mode == Mode::Grid ? 4.0 : 0.35;
        const qreal alignedSecondary = secondary <= m_rowTolerance ? 0.0 : secondary;
        const qreal score = primary + alignedSecondary * secondaryWeight;
        if (score < bestScore) {
            bestScore = score;
            best = candidate;
        }
    }

    return best ? focusItem(best, Qt::OtherFocusReason) : false;
}

bool FocusNavigator::focusItem(QQuickItem *item, Qt::FocusReason reason)
{
    if (!isFocusable(item) || !isInScope(item))
        return false;
    item->forceActiveFocus(reason);
    emit currentFocusChanged();
    return true;
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

QList<QQuickItem *> FocusNavigator::focusableItems() const
{
    QList<QQuickItem *> result;
    if (!m_scope)
        return result;

    const auto collect = [&](auto &&self, QQuickItem *parent) -> void {
        for (QQuickItem *child : parent->childItems()) {
            if (isFocusable(child))
                result.push_back(child);
            self(self, child);
        }
    };
    collect(collect, m_scope);
    return result;
}

QList<QQuickItem *> FocusNavigator::orderedItems() const
{
    if (m_mode == Mode::Native) {
        QList<QQuickItem *> result;
        if (!m_scope)
            return result;

        QQuickItem *candidate = m_scope->nextItemInFocusChain(true);
        QQuickItem *start = candidate;
        int guard = 0;
        while (candidate && guard++ < 4096) {
            if (candidate == start && guard > 1)
                break;
            if (isInScope(candidate) && isFocusable(candidate))
                result.push_back(candidate);
            candidate = candidate->nextItemInFocusChain(true);
        }
        return result;
    }

    auto items = focusableItems();
    std::stable_sort(items.begin(), items.end(), [this](QQuickItem *left, QQuickItem *right) {
        if (m_mode == Mode::Explicit) {
            const int leftOrder = explicitOrder(left);
            const int rightOrder = explicitOrder(right);
            const int normalizedLeft = leftOrder == 0 ? std::numeric_limits<int>::max() : leftOrder;
            const int normalizedRight = rightOrder == 0 ? std::numeric_limits<int>::max() : rightOrder;
            if (normalizedLeft != normalizedRight)
                return normalizedLeft < normalizedRight;
        }

        const QPointF leftCenter = itemCenter(left);
        const QPointF rightCenter = itemCenter(right);
        if (std::abs(leftCenter.y() - rightCenter.y()) > m_rowTolerance)
            return leftCenter.y() < rightCenter.y();
        if (!qFuzzyCompare(leftCenter.x(), rightCenter.x()))
            return leftCenter.x() < rightCenter.x();
        return left < right;
    });
    return items;
}

QPointF FocusNavigator::itemCenter(QQuickItem *item) const
{
    if (!item || !m_scope)
        return {};
    return item->mapToItem(m_scope, QPointF(item->width() / 2.0, item->height() / 2.0));
}

int FocusNavigator::explicitOrder(QQuickItem *item) const
{
    if (!item)
        return 0;
    if (auto *attached = qmlAttachedPropertiesObject<Lf>(item, false))
        return attached->focusOrder();
    return 0;
}

void FocusNavigator::reconnectWindow()
{
    QObject::disconnect(m_focusConnection);
    QObject::disconnect(m_windowDestroyedConnection);
    m_focusConnection = {};
    m_windowDestroyedConnection = {};

    QQuickWindow *window = m_scope ? m_scope->window() : nullptr;
    if (!window)
        return;

    m_focusConnection = connect(window, &QQuickWindow::activeFocusItemChanged,
                                this, &FocusNavigator::currentFocusChanged);
    m_windowDestroyedConnection = connect(window, &QObject::destroyed, this, [this] {
        m_focusConnection = {};
        m_windowDestroyedConnection = {};
        emit currentFocusChanged();
    });
}

} // namespace LfQuick
