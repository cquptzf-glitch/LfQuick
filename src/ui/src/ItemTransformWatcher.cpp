#include <LfQuick/Ui/ItemTransformWatcher.h>

#include <QQuickWindow>
#include <QSet>

#include <utility>

namespace LfQuick {

ItemTransformWatcher::ItemTransformWatcher(QObject *parent)
    : QObject(parent)
{
}

ItemTransformWatcher::~ItemTransformWatcher()
{
    clearConnections();
}

void ItemTransformWatcher::setSource(QQuickItem *item)
{
    if (m_source == item)
        return;
    m_source = item;
    rebuild();
    emit sourceChanged();
    invalidate();
}

void ItemTransformWatcher::setTarget(QQuickItem *item)
{
    if (m_target == item)
        return;
    m_target = item;
    rebuild();
    emit targetChanged();
    invalidate();
}

void ItemTransformWatcher::setCommonParent(QQuickItem *item)
{
    if (m_commonParent == item)
        return;
    m_commonParent = item;
    rebuild();
    emit commonParentChanged();
    invalidate();
}

void ItemTransformWatcher::refresh()
{
    rebuild();
    invalidate();
}

void ItemTransformWatcher::rebuild()
{
    clearConnections();
    m_watchedItems.clear();
    m_watchedWindows.clear();

    addItemChain(m_source);
    addItemChain(m_target);

    // A supplied commonParent is also watched even if a temporarily detached
    // source/target has not yet reached it. This makes transient reparenting
    // during QML construction settle correctly on the next geometry change.
    if (m_commonParent)
        watchItem(m_commonParent);

    for (const QPointer<QQuickItem> &item : std::as_const(m_watchedItems)) {
        if (item && item->window())
            watchWindow(item->window());
    }
}

void ItemTransformWatcher::clearConnections()
{
    for (const QMetaObject::Connection &connection : std::as_const(m_connections))
        QObject::disconnect(connection);
    m_connections.clear();
}

void ItemTransformWatcher::addItemChain(QQuickItem *item)
{
    QSet<QQuickItem *> visited;
    for (QQuickItem *cursor = item; cursor && !visited.contains(cursor);
         cursor = cursor->parentItem()) {
        visited.insert(cursor);
        watchItem(cursor);
        if (cursor == m_commonParent)
            break;
    }
}

void ItemTransformWatcher::watchItem(QQuickItem *item)
{
    if (!item)
        return;
    for (const QPointer<QQuickItem> &existing : std::as_const(m_watchedItems)) {
        if (existing == item)
            return;
    }
    m_watchedItems.push_back(item);

    const bool endpointSource = item == m_source;
    const bool endpointTarget = item == m_target;
    const bool endpointCommonParent = item == m_commonParent;

    const auto changed = [this] { invalidate(); };
    const auto topologyChanged = [this] {
        rebuild();
        invalidate();
    };

    m_connections.push_back(connect(item, &QQuickItem::xChanged, this, changed));
    m_connections.push_back(connect(item, &QQuickItem::yChanged, this, changed));
    m_connections.push_back(connect(item, &QQuickItem::widthChanged, this, changed));
    m_connections.push_back(connect(item, &QQuickItem::heightChanged, this, changed));
    m_connections.push_back(connect(item, &QQuickItem::scaleChanged, this, changed));
    m_connections.push_back(connect(item, &QQuickItem::rotationChanged, this, changed));
    m_connections.push_back(connect(item, &QQuickItem::transformOriginChanged, this, changed));
    m_connections.push_back(connect(item, &QQuickItem::parentChanged, this,
                                    [topologyChanged](QQuickItem *) { topologyChanged(); }));
    m_connections.push_back(connect(item, &QQuickItem::windowChanged, this,
                                    [topologyChanged](QQuickWindow *) { topologyChanged(); }));
    m_connections.push_back(connect(
        item, &QObject::destroyed, this,
        [this, endpointSource, endpointTarget, endpointCommonParent] {
            // QPointer is already null by QObject::destroyed emission time, so
            // remember the endpoint roles when the connection is established.
            if (endpointSource) {
                m_source = nullptr;
                emit sourceChanged();
            }
            if (endpointTarget) {
                m_target = nullptr;
                emit targetChanged();
            }
            if (endpointCommonParent) {
                m_commonParent = nullptr;
                emit commonParentChanged();
            }
            rebuild();
            invalidate();
        }));
}

void ItemTransformWatcher::watchWindow(QQuickWindow *window)
{
    if (!window)
        return;
    for (const QPointer<QQuickWindow> &existing : std::as_const(m_watchedWindows)) {
        if (existing == window)
            return;
    }
    m_watchedWindows.push_back(window);

    const auto changed = [this] { invalidate(); };
    m_connections.push_back(connect(window, &QWindow::xChanged, this, changed));
    m_connections.push_back(connect(window, &QWindow::yChanged, this, changed));
    m_connections.push_back(connect(window, &QWindow::widthChanged, this, changed));
    m_connections.push_back(connect(window, &QWindow::heightChanged, this, changed));
    m_connections.push_back(connect(window, &QWindow::screenChanged, this,
                                    [changed](QScreen *) { changed(); }));
    m_connections.push_back(connect(window, &QObject::destroyed, this, [this] {
        rebuild();
        invalidate();
    }));
}

void ItemTransformWatcher::invalidate()
{
    ++m_revision;
    emit transformChanged();
}

} // namespace LfQuick
