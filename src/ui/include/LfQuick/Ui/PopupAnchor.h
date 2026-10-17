#pragma once

#include <LfQuick/Ui/LfQuickUiGlobal.h>

#include <QObject>
#include <QPointF>
#include <QPointer>
#include <QQuickItem>
#include <QRectF>
#include <QSizeF>
#include <QtQmlIntegration/qqmlintegration.h>

namespace LfQuick {

class ItemTransformWatcher;

class LFQUICKUI_EXPORT PopupAnchor : public QObject
{
    Q_OBJECT
    QML_NAMED_ELEMENT(LfPopupAnchor)
    Q_PROPERTY(QQuickItem* anchorItem READ anchorItem WRITE setAnchorItem NOTIFY anchorItemChanged FINAL)
    Q_PROPERTY(QQuickItem* parentItem READ parentItem WRITE setParentItem NOTIFY parentItemChanged FINAL)
    Q_PROPERTY(QSizeF popupSize READ popupSize WRITE setPopupSize NOTIFY popupSizeChanged FINAL)
    Q_PROPERTY(QRectF availableRect READ availableRect WRITE setAvailableRect NOTIFY availableRectChanged FINAL)
    Q_PROPERTY(Edge edge READ edge WRITE setEdge NOTIFY policyChanged FINAL)
    Q_PROPERTY(Alignment alignment READ alignment WRITE setAlignment NOTIFY policyChanged FINAL)
    Q_PROPERTY(QPointF offset READ offset WRITE setOffset NOTIFY policyChanged FINAL)
    Q_PROPERTY(qreal margin READ margin WRITE setMargin NOTIFY policyChanged FINAL)
    Q_PROPERTY(bool flip READ flip WRITE setFlip NOTIFY policyChanged FINAL)
    Q_PROPERTY(QPointF position READ position NOTIFY positionChanged FINAL)
    Q_PROPERTY(Edge resolvedEdge READ resolvedEdge NOTIFY positionChanged FINAL)

public:
    enum class Edge {
        Auto,
        Top,
        Bottom,
        Left,
        Right
    };
    Q_ENUM(Edge)

    enum class Alignment {
        Start,
        Center,
        End
    };
    Q_ENUM(Alignment)

    explicit PopupAnchor(QObject *parent = nullptr);

    QQuickItem *anchorItem() const noexcept { return m_anchorItem; }
    void setAnchorItem(QQuickItem *item);

    QQuickItem *parentItem() const noexcept { return m_parentItem; }
    void setParentItem(QQuickItem *item);

    QSizeF popupSize() const noexcept { return m_popupSize; }
    void setPopupSize(const QSizeF &size);

    QRectF availableRect() const noexcept { return m_availableRect; }
    void setAvailableRect(const QRectF &rect);

    Edge edge() const noexcept { return m_edge; }
    void setEdge(Edge edge);

    Alignment alignment() const noexcept { return m_alignment; }
    void setAlignment(Alignment alignment);

    QPointF offset() const noexcept { return m_offset; }
    void setOffset(const QPointF &offset);

    qreal margin() const noexcept { return m_margin; }
    void setMargin(qreal margin);

    bool flip() const noexcept { return m_flip; }
    void setFlip(bool flip);

    QPointF position() const noexcept { return m_position; }
    Edge resolvedEdge() const noexcept { return m_resolvedEdge; }

    Q_INVOKABLE QPointF positionFor(const QSizeF &size);
    Q_INVOKABLE void refresh();

signals:
    void anchorItemChanged();
    void parentItemChanged();
    void popupSizeChanged();
    void availableRectChanged();
    void policyChanged();
    void positionChanged();

private:
    struct Placement {
        QPointF position;
        Edge edge = Edge::Bottom;
    };

    void reconnectAnchor();
    void reconnectParent();
    void disconnectAnchor();
    void disconnectParent();
    void syncTransformWatcher();
    QRectF effectiveAvailableRect() const;
    QRectF anchorRect() const;
    Placement calculate(const QSizeF &size) const;
    QPointF candidatePosition(Edge edge,
                              const QRectF &anchor,
                              const QSizeF &size) const;
    qreal primaryOverflow(Edge edge,
                          const QPointF &position,
                          const QSizeF &size,
                          const QRectF &available) const;
    static Edge opposite(Edge edge);
    static QPointF clampPosition(QPointF position,
                                 const QSizeF &size,
                                 const QRectF &available);

    QPointer<QQuickItem> m_anchorItem;
    QPointer<QQuickItem> m_parentItem;
    QList<QMetaObject::Connection> m_anchorConnections;
    QList<QMetaObject::Connection> m_parentConnections;
    ItemTransformWatcher *m_transformWatcher = nullptr;
    QSizeF m_popupSize;
    QRectF m_availableRect;
    Edge m_edge = Edge::Auto;
    Alignment m_alignment = Alignment::Start;
    QPointF m_offset;
    qreal m_margin = 4.0;
    bool m_flip = true;
    QPointF m_position;
    Edge m_resolvedEdge = Edge::Bottom;
};

} // namespace LfQuick
