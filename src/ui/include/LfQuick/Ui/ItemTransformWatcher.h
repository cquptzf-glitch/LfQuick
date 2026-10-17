#pragma once

#include <LfQuick/Ui/LfQuickUiGlobal.h>

#include <QObject>
#include <QPointer>
#include <QQuickItem>
#include <QQuickWindow>
#include <QtQmlIntegration/qqmlintegration.h>

namespace LfQuick {

// Watches every QQuickItem/window on the geometry path between two items.
// This is intentionally public because popup anchoring, overlays, drag/drop,
// focus adorners and QWidget/Quick bridges all need the same invalidation
// primitive when an ancestor moves, scales, rotates or is reparented.
class LFQUICKUI_EXPORT ItemTransformWatcher final : public QObject
{
    Q_OBJECT
    QML_NAMED_ELEMENT(LfTransformWatcher)

    Q_PROPERTY(QQuickItem* source READ source WRITE setSource NOTIFY sourceChanged FINAL)
    Q_PROPERTY(QQuickItem* target READ target WRITE setTarget NOTIFY targetChanged FINAL)
    Q_PROPERTY(QQuickItem* commonParent READ commonParent WRITE setCommonParent NOTIFY commonParentChanged FINAL)
    Q_PROPERTY(quint64 revision READ revision NOTIFY transformChanged FINAL)

public:
    explicit ItemTransformWatcher(QObject *parent = nullptr);
    ~ItemTransformWatcher() override;

    QQuickItem *source() const noexcept { return m_source; }
    void setSource(QQuickItem *item);

    QQuickItem *target() const noexcept { return m_target; }
    void setTarget(QQuickItem *item);

    QQuickItem *commonParent() const noexcept { return m_commonParent; }
    void setCommonParent(QQuickItem *item);

    quint64 revision() const noexcept { return m_revision; }

    Q_INVOKABLE void refresh();

signals:
    void sourceChanged();
    void targetChanged();
    void commonParentChanged();
    void transformChanged();

private:
    void rebuild();
    void clearConnections();
    void addItemChain(QQuickItem *item);
    void watchItem(QQuickItem *item);
    void watchWindow(QQuickWindow *window);
    void invalidate();

    QPointer<QQuickItem> m_source;
    QPointer<QQuickItem> m_target;
    QPointer<QQuickItem> m_commonParent;
    QList<QPointer<QQuickItem>> m_watchedItems;
    QList<QPointer<QQuickWindow>> m_watchedWindows;
    QList<QMetaObject::Connection> m_connections;
    quint64 m_revision = 0;
};

} // namespace LfQuick
