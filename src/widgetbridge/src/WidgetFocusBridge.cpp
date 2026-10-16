#include <LfQuick/WidgetBridge/WidgetFocusBridge.h>

#include <QApplication>
#include <QEvent>
#include <QKeyEvent>
#include <QQuickItem>
#include <QQuickWidget>
#include <QQuickWindow>
#include <QWidget>

namespace LfQuick {

WidgetFocusBridge::WidgetFocusBridge(QQuickWidget *surface, QObject *parent)
    : QObject(parent)
    , m_surface(surface)
{
    if (m_surface)
        m_surface->installEventFilter(this);
}

WidgetFocusBridge::~WidgetFocusBridge()
{
    if (m_surface)
        m_surface->removeEventFilter(this);
    if (m_before)
        m_before->removeEventFilter(this);
    if (m_after && m_after != m_before)
        m_after->removeEventFilter(this);
}

void WidgetFocusBridge::setPolicy(FocusTransferPolicy value)
{
    if (m_policy == value)
        return;
    m_policy = value;
    emit policyChanged();
}

void WidgetFocusBridge::setScannerIsolation(bool value)
{
    if (m_scannerIsolation == value)
        return;
    m_scannerIsolation = value;
    emit scannerIsolationChanged();
}

void WidgetFocusBridge::setModalFocusTrap(bool value)
{
    if (m_modalFocusTrap == value)
        return;
    m_modalFocusTrap = value;
    emit modalFocusTrapChanged();
}

void WidgetFocusBridge::setBoundaryWidgets(QWidget *before, QWidget *after)
{
    if (m_before)
        m_before->removeEventFilter(this);
    if (m_after && m_after != m_before)
        m_after->removeEventFilter(this);

    m_before = before;
    m_after = after;

    if (m_before)
        m_before->installEventFilter(this);
    if (m_after && m_after != m_before)
        m_after->installEventFilter(this);
}

void WidgetFocusBridge::setQuickBoundaryItems(QQuickItem *entry, QQuickItem *exit)
{
    m_entryItem = entry;
    m_exitItem = exit;
}

void WidgetFocusBridge::rememberFocus()
{
    if (auto *widget = QApplication::focusWidget()) {
        if (widget != m_surface && (!m_surface || !m_surface->isAncestorOf(widget)))
            m_lastWidgetFocus = widget;
    }

    if (m_surface && m_surface->quickWindow())
        m_lastQuickFocus = m_surface->quickWindow()->activeFocusItem();
}

bool WidgetFocusBridge::restoreFocus()
{
    if (!canTransfer())
        return false;

    if (m_lastQuickFocus && m_surface && m_surface->isVisible())
        return focusQuickItem(m_lastQuickFocus, Qt::OtherFocusReason);

    if (m_lastWidgetFocus && m_lastWidgetFocus->isVisible() && m_lastWidgetFocus->isEnabled())
        return focusWidget(m_lastWidgetFocus, Qt::OtherFocusReason);

    return false;
}

bool WidgetFocusBridge::eventFilter(QObject *watched, QEvent *event)
{
    if (m_policy == FocusTransferPolicy::Disabled || !event)
        return QObject::eventFilter(watched, event);

    if (event->type() == QEvent::FocusIn)
        rememberFocus();

    if (event->type() != QEvent::KeyPress)
        return QObject::eventFilter(watched, event);

    auto *key = static_cast<QKeyEvent *>(event);
    if (watched == m_surface)
        return handleQuickKey(key) || QObject::eventFilter(watched, event);

    auto *widget = qobject_cast<QWidget *>(watched);
    return (widget && handleWidgetKey(widget, key)) || QObject::eventFilter(watched, event);
}

bool WidgetFocusBridge::canTransfer() const
{
    if (m_policy == FocusTransferPolicy::Disabled || m_scannerIsolation)
        return false;

    if (m_modalFocusTrap && QApplication::activeModalWidget())
        return false;

    return true;
}

bool WidgetFocusBridge::handleWidgetKey(QWidget *widget, QKeyEvent *event)
{
    if (!widget || !event || !canTransfer())
        return false;

    const bool backtab = event->key() == Qt::Key_Backtab
        || (event->key() == Qt::Key_Tab && event->modifiers().testFlag(Qt::ShiftModifier));
    const bool tab = event->key() == Qt::Key_Tab && !backtab;

    if (widget == m_before && tab) {
        if (focusQuickItem(m_entryItem, Qt::TabFocusReason)) {
            emit focusTransferred(QStringLiteral("widget-to-quick"));
            return true;
        }
    }

    if (widget == m_after && backtab) {
        if (focusQuickItem(m_exitItem, Qt::BacktabFocusReason)) {
            emit focusTransferred(QStringLiteral("widget-to-quick-backtab"));
            return true;
        }
    }

    return false;
}

bool WidgetFocusBridge::handleQuickKey(QKeyEvent *event)
{
    if (!event || !canTransfer() || !m_surface || !m_surface->quickWindow())
        return false;

    QQuickItem *activeItem = m_surface->quickWindow()->activeFocusItem();
    const bool backtab = event->key() == Qt::Key_Backtab
        || (event->key() == Qt::Key_Tab && event->modifiers().testFlag(Qt::ShiftModifier));
    const bool tab = event->key() == Qt::Key_Tab && !backtab;

    if (tab && activeItem == m_exitItem) {
        if (focusWidget(m_after, Qt::TabFocusReason)) {
            emit focusTransferred(QStringLiteral("quick-to-widget"));
            return true;
        }
    }

    if (backtab && activeItem == m_entryItem) {
        if (focusWidget(m_before, Qt::BacktabFocusReason)) {
            emit focusTransferred(QStringLiteral("quick-to-widget-backtab"));
            return true;
        }
    }

    return false;
}

bool WidgetFocusBridge::focusQuickItem(QQuickItem *item, Qt::FocusReason reason)
{
    if (!item || !m_surface || !m_surface->isEnabled() || !m_surface->isVisible())
        return false;

    m_surface->setFocus(reason);
    item->forceActiveFocus(reason);
    m_lastQuickFocus = item;
    return m_surface->hasFocus();
}

bool WidgetFocusBridge::focusWidget(QWidget *widget, Qt::FocusReason reason)
{
    if (!widget || !widget->isEnabled() || !widget->isVisible())
        return false;

    widget->setFocus(reason);
    m_lastWidgetFocus = widget;
    return widget->hasFocus();
}

} // namespace LfQuick
