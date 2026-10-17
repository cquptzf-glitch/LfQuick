#include <LfQuick/Ui/FocusWalker.h>

#include <LfQuick/Ui/LfAttached.h>

#include <QtQml/qqml.h>

#include <algorithm>
#include <cmath>
#include <limits>

namespace LfQuick {

FocusWalker::FocusWalker(QObject *parent)
    : QObject(parent)
{
}

void FocusWalker::setScope(QQuickItem *scope)
{
    if (m_scope == scope)
        return;

    QObject::disconnect(m_scopeDestroyedConnection);
    m_scopeDestroyedConnection = {};
    m_scope = scope;

    if (m_scope) {
        m_scopeDestroyedConnection = connect(m_scope, &QObject::destroyed, this, [this] {
            m_scope = nullptr;
            m_scopeDestroyedConnection = {};
            emit scopeChanged();
        });
    }

    emit scopeChanged();
}

void FocusWalker::setRowTolerance(qreal value)
{
    value = std::max<qreal>(0.0, value);
    if (qFuzzyCompare(m_rowTolerance, value))
        return;
    m_rowTolerance = value;
    emit policyChanged();
}

void FocusWalker::setLayoutDirection(Qt::LayoutDirection direction)
{
    if (m_layoutDirection == direction)
        return;
    m_layoutDirection = direction;
    emit policyChanged();
}

QQuickItem *FocusWalker::next(QQuickItem *current, Mode mode, bool forward, bool wrap) const
{
    const auto items = orderedItems(mode);
    if (items.isEmpty())
        return nullptr;

    if (!current || !items.contains(current))
        return forward ? items.first() : items.last();

    const int index = items.indexOf(current);
    int candidate = index + (forward ? 1 : -1);
    if (candidate < 0 || candidate >= items.size()) {
        if (!wrap)
            return nullptr;
        candidate = forward ? 0 : items.size() - 1;
    }
    return items.at(candidate);
}

QQuickItem *FocusWalker::directional(QQuickItem *current,
                                     Direction direction,
                                     Mode mode) const
{
    const auto items = collectFocusable();
    if (items.isEmpty())
        return nullptr;
    if (!current || !contains(current))
        return orderedItems(mode).value(0, nullptr);

    const QPointF origin = centerInScope(current);
    QQuickItem *best = nullptr;
    qreal bestScore = std::numeric_limits<qreal>::max();

    for (QQuickItem *candidate : items) {
        if (candidate == current)
            continue;

        const QPointF center = centerInScope(candidate);
        const qreal dx = center.x() - origin.x();
        const qreal dy = center.y() - origin.y();

        qreal primary = 0.0;
        qreal secondary = 0.0;
        bool valid = false;
        switch (direction) {
        case Direction::Left:
            valid = dx < 0.0;
            primary = -dx;
            secondary = std::abs(dy);
            break;
        case Direction::Right:
            valid = dx > 0.0;
            primary = dx;
            secondary = std::abs(dy);
            break;
        case Direction::Up:
            valid = dy < 0.0;
            primary = -dy;
            secondary = std::abs(dx);
            break;
        case Direction::Down:
            valid = dy > 0.0;
            primary = dy;
            secondary = std::abs(dx);
            break;
        }
        if (!valid)
            continue;

        const qreal alignedSecondary = secondary <= m_rowTolerance ? 0.0 : secondary;
        const qreal weight = mode == Mode::Grid ? 4.0 : 0.35;
        const qreal explicitBias = explicitOrder(candidate) > 0
            ? qreal(explicitOrder(candidate)) * 0.0001
            : 0.0;
        const qreal score = primary + alignedSecondary * weight + explicitBias;
        if (score < bestScore) {
            bestScore = score;
            best = candidate;
        }
    }

    return best;
}

QVariantList FocusWalker::dumpOrder(Mode mode) const
{
    QVariantList result;
    const auto items = orderedItems(mode);
    result.reserve(items.size());
    for (int i = 0; i < items.size(); ++i) {
        QQuickItem *item = items.at(i);
        const QPointF center = centerInScope(item);
        result.push_back(QVariantMap{
            {QStringLiteral("index"), i},
            {QStringLiteral("item"), QVariant::fromValue(static_cast<QObject *>(item))},
            {QStringLiteral("name"), item->objectName().isEmpty()
                ? QString::fromLatin1(item->metaObject()->className())
                : item->objectName()},
            {QStringLiteral("focusOrder"), explicitOrder(item)},
            {QStringLiteral("x"), center.x()},
            {QStringLiteral("y"), center.y()},
            {QStringLiteral("width"), item->width()},
            {QStringLiteral("height"), item->height()}
        });
    }
    return result;
}

bool FocusWalker::contains(QQuickItem *item) const
{
    return item && m_scope && item != m_scope && m_scope->isAncestorOf(item);
}

bool FocusWalker::focusable(QQuickItem *item) const
{
    if (!item || !contains(item) || !item->isVisible() || !item->isEnabled())
        return false;
    return item->focusPolicy() != Qt::NoFocus || item->activeFocusOnTab();
}

QList<QQuickItem *> FocusWalker::collectFocusable() const
{
    QList<QQuickItem *> result;
    if (!m_scope)
        return result;

    const auto collect = [&](auto &&self, QQuickItem *parent) -> void {
        for (QQuickItem *child : parent->childItems()) {
            if (focusable(child))
                result.push_back(child);
            self(self, child);
        }
    };
    collect(collect, m_scope);
    return result;
}

QList<QQuickItem *> FocusWalker::orderedItems(Mode mode) const
{
    if (!m_scope)
        return {};

    if (mode == Mode::Native) {
        QList<QQuickItem *> result;
        QQuickItem *candidate = m_scope->nextItemInFocusChain(true);
        QQuickItem *start = candidate;
        int guard = 0;
        while (candidate && guard++ < 4096) {
            if (candidate == start && guard > 1)
                break;
            if (focusable(candidate))
                result.push_back(candidate);
            candidate = candidate->nextItemInFocusChain(true);
        }
        return result;
    }

    auto items = collectFocusable();
    std::stable_sort(items.begin(), items.end(), [this, mode](QQuickItem *left, QQuickItem *right) {
        if (mode == Mode::Explicit) {
            const int leftOrder = explicitOrder(left);
            const int rightOrder = explicitOrder(right);
            const int normalizedLeft = leftOrder > 0 ? leftOrder : std::numeric_limits<int>::max();
            const int normalizedRight = rightOrder > 0 ? rightOrder : std::numeric_limits<int>::max();
            if (normalizedLeft != normalizedRight)
                return normalizedLeft < normalizedRight;
        }

        const QPointF leftCenter = centerInScope(left);
        const QPointF rightCenter = centerInScope(right);
        if (std::abs(leftCenter.y() - rightCenter.y()) > m_rowTolerance)
            return leftCenter.y() < rightCenter.y();
        if (!qFuzzyCompare(leftCenter.x(), rightCenter.x())) {
            return m_layoutDirection == Qt::RightToLeft
                ? leftCenter.x() > rightCenter.x()
                : leftCenter.x() < rightCenter.x();
        }
        return false;
    });
    return items;
}

QPointF FocusWalker::centerInScope(QQuickItem *item) const
{
    if (!item || !m_scope)
        return {};
    return item->mapToItem(m_scope, QPointF(item->width() / 2.0, item->height() / 2.0));
}

int FocusWalker::explicitOrder(QQuickItem *item) const
{
    if (!item)
        return 0;
    if (auto *attached = qmlAttachedPropertiesObject<Lf>(item, false))
        return attached->focusOrder();
    return 0;
}

} // namespace LfQuick
