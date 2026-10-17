#include <LfQuick/Ui/PopupAnchor.h>

#include <QQuickWindow>

#include <algorithm>
#include <utility>

namespace LfQuick {
namespace {

QQuickItem *coordinateParent(QQuickItem *anchor, QQuickItem *explicitParent)
{
    if (explicitParent)
        return explicitParent;
    if (anchor && anchor->window())
        return anchor->window()->contentItem();
    return anchor ? anchor->parentItem() : nullptr;
}

} // namespace

PopupAnchor::PopupAnchor(QObject *parent)
    : QObject(parent)
{
}

void PopupAnchor::setAnchorItem(QQuickItem *item)
{
    if (m_anchorItem == item)
        return;
    disconnectAnchor();
    m_anchorItem = item;
    reconnectAnchor();
    emit anchorItemChanged();
    refresh();
}

void PopupAnchor::setParentItem(QQuickItem *item)
{
    if (m_parentItem == item)
        return;
    disconnectParent();
    m_parentItem = item;
    reconnectParent();
    emit parentItemChanged();
    refresh();
}

void PopupAnchor::setPopupSize(const QSizeF &size)
{
    const QSizeF normalized(std::max<qreal>(0.0, size.width()),
                            std::max<qreal>(0.0, size.height()));
    if (m_popupSize == normalized)
        return;
    m_popupSize = normalized;
    emit popupSizeChanged();
    refresh();
}

void PopupAnchor::setAvailableRect(const QRectF &rect)
{
    if (m_availableRect == rect)
        return;
    m_availableRect = rect;
    emit availableRectChanged();
    refresh();
}

void PopupAnchor::setEdge(Edge edge)
{
    if (m_edge == edge)
        return;
    m_edge = edge;
    emit policyChanged();
    refresh();
}

void PopupAnchor::setAlignment(Alignment alignment)
{
    if (m_alignment == alignment)
        return;
    m_alignment = alignment;
    emit policyChanged();
    refresh();
}

void PopupAnchor::setOffset(const QPointF &offset)
{
    if (m_offset == offset)
        return;
    m_offset = offset;
    emit policyChanged();
    refresh();
}

void PopupAnchor::setMargin(qreal margin)
{
    margin = std::max<qreal>(0.0, margin);
    if (qFuzzyCompare(m_margin, margin))
        return;
    m_margin = margin;
    emit policyChanged();
    refresh();
}

void PopupAnchor::setFlip(bool flip)
{
    if (m_flip == flip)
        return;
    m_flip = flip;
    emit policyChanged();
    refresh();
}

QPointF PopupAnchor::positionFor(const QSizeF &size)
{
    return calculate(size).position;
}

void PopupAnchor::refresh()
{
    const Placement next = calculate(m_popupSize);
    if (m_position == next.position && m_resolvedEdge == next.edge)
        return;
    m_position = next.position;
    m_resolvedEdge = next.edge;
    emit positionChanged();
}

void PopupAnchor::reconnectAnchor()
{
    if (!m_anchorItem)
        return;

    const auto changed = [this] { refresh(); };
    m_anchorConnections.push_back(connect(m_anchorItem, &QQuickItem::xChanged,
                                          this, changed));
    m_anchorConnections.push_back(connect(m_anchorItem, &QQuickItem::yChanged,
                                          this, changed));
    m_anchorConnections.push_back(connect(m_anchorItem, &QQuickItem::widthChanged,
                                          this, changed));
    m_anchorConnections.push_back(connect(m_anchorItem, &QQuickItem::heightChanged,
                                          this, changed));
    m_anchorConnections.push_back(connect(m_anchorItem, &QQuickItem::windowChanged,
                                          this, [this](QQuickWindow *) { refresh(); }));
    m_anchorConnections.push_back(connect(m_anchorItem, &QObject::destroyed, this, [this] {
        m_anchorConnections.clear();
        m_anchorItem = nullptr;
        emit anchorItemChanged();
        refresh();
    }));
}

void PopupAnchor::reconnectParent()
{
    if (!m_parentItem)
        return;

    const auto changed = [this] { refresh(); };
    m_parentConnections.push_back(connect(m_parentItem, &QQuickItem::widthChanged,
                                          this, changed));
    m_parentConnections.push_back(connect(m_parentItem, &QQuickItem::heightChanged,
                                          this, changed));
    m_parentConnections.push_back(connect(m_parentItem, &QObject::destroyed, this, [this] {
        m_parentConnections.clear();
        m_parentItem = nullptr;
        emit parentItemChanged();
        refresh();
    }));
}

void PopupAnchor::disconnectAnchor()
{
    for (const QMetaObject::Connection &connection : std::as_const(m_anchorConnections))
        QObject::disconnect(connection);
    m_anchorConnections.clear();
}

void PopupAnchor::disconnectParent()
{
    for (const QMetaObject::Connection &connection : std::as_const(m_parentConnections))
        QObject::disconnect(connection);
    m_parentConnections.clear();
}

QRectF PopupAnchor::effectiveAvailableRect() const
{
    QRectF rect = m_availableRect;
    if (!rect.isValid() || rect.isEmpty()) {
        QQuickItem *parent = coordinateParent(m_anchorItem, m_parentItem);
        if (!parent)
            return {};
        rect = QRectF(0.0, 0.0, parent->width(), parent->height());
    }

    const qreal left = rect.left() + m_margin;
    const qreal top = rect.top() + m_margin;
    const qreal right = std::max(left, rect.right() - m_margin);
    const qreal bottom = std::max(top, rect.bottom() - m_margin);
    return QRectF(QPointF(left, top), QPointF(right, bottom));
}

QRectF PopupAnchor::anchorRect() const
{
    if (!m_anchorItem)
        return {};

    QQuickItem *parent = coordinateParent(m_anchorItem, m_parentItem);
    if (!parent)
        return QRectF(m_anchorItem->x(), m_anchorItem->y(),
                      m_anchorItem->width(), m_anchorItem->height());

    const QPointF topLeft = m_anchorItem->mapToItem(parent, QPointF(0.0, 0.0));
    const QPointF bottomRight = m_anchorItem->mapToItem(
        parent, QPointF(m_anchorItem->width(), m_anchorItem->height()));
    return QRectF(topLeft, bottomRight).normalized();
}

PopupAnchor::Placement PopupAnchor::calculate(const QSizeF &rawSize) const
{
    const QSizeF size(std::max<qreal>(0.0, rawSize.width()),
                      std::max<qreal>(0.0, rawSize.height()));
    const QRectF anchor = anchorRect();
    const QRectF available = effectiveAvailableRect();

    Edge chosen = m_edge == Edge::Auto ? Edge::Bottom : m_edge;
    QPointF candidate = candidatePosition(chosen, anchor, size);

    if (m_flip && available.isValid() && !available.isEmpty()) {
        const Edge alternative = opposite(chosen);
        const QPointF alternativePosition = candidatePosition(alternative, anchor, size);
        const qreal overflow = primaryOverflow(chosen, candidate, size, available);
        const qreal alternativeOverflow = primaryOverflow(
            alternative, alternativePosition, size, available);
        if (alternativeOverflow < overflow) {
            chosen = alternative;
            candidate = alternativePosition;
        }
    }

    if (available.isValid() && !available.isEmpty())
        candidate = clampPosition(candidate, size, available);

    return {candidate, chosen};
}

QPointF PopupAnchor::candidatePosition(Edge edge,
                                       const QRectF &anchor,
                                       const QSizeF &size) const
{
    qreal x = anchor.left();
    qreal y = anchor.bottom();

    const auto aligned = [this](qreal start, qreal extent, qreal popupExtent) {
        switch (m_alignment) {
        case Alignment::Start:
            return start;
        case Alignment::Center:
            return start + (extent - popupExtent) / 2.0;
        case Alignment::End:
            return start + extent - popupExtent;
        }
        return start;
    };

    switch (edge) {
    case Edge::Auto:
    case Edge::Bottom:
        x = aligned(anchor.left(), anchor.width(), size.width());
        y = anchor.bottom();
        break;
    case Edge::Top:
        x = aligned(anchor.left(), anchor.width(), size.width());
        y = anchor.top() - size.height();
        break;
    case Edge::Left:
        x = anchor.left() - size.width();
        y = aligned(anchor.top(), anchor.height(), size.height());
        break;
    case Edge::Right:
        x = anchor.right();
        y = aligned(anchor.top(), anchor.height(), size.height());
        break;
    }

    return QPointF(x + m_offset.x(), y + m_offset.y());
}

qreal PopupAnchor::primaryOverflow(Edge edge,
                                   const QPointF &position,
                                   const QSizeF &size,
                                   const QRectF &available) const
{
    switch (edge) {
    case Edge::Auto:
    case Edge::Bottom:
        return std::max<qreal>(0.0, position.y() + size.height() - available.bottom());
    case Edge::Top:
        return std::max<qreal>(0.0, available.top() - position.y());
    case Edge::Left:
        return std::max<qreal>(0.0, available.left() - position.x());
    case Edge::Right:
        return std::max<qreal>(0.0, position.x() + size.width() - available.right());
    }
    return 0.0;
}

PopupAnchor::Edge PopupAnchor::opposite(Edge edge)
{
    switch (edge) {
    case Edge::Top:
        return Edge::Bottom;
    case Edge::Bottom:
    case Edge::Auto:
        return Edge::Top;
    case Edge::Left:
        return Edge::Right;
    case Edge::Right:
        return Edge::Left;
    }
    return Edge::Bottom;
}

QPointF PopupAnchor::clampPosition(QPointF position,
                                   const QSizeF &size,
                                   const QRectF &available)
{
    const qreal maxX = std::max(available.left(), available.right() - size.width());
    const qreal maxY = std::max(available.top(), available.bottom() - size.height());
    position.setX(std::clamp(position.x(), available.left(), maxX));
    position.setY(std::clamp(position.y(), available.top(), maxY));
    return position;
}

} // namespace LfQuick
