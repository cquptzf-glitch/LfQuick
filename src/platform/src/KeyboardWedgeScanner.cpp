#include <LfQuick/Platform/KeyboardWedgeScanner.h>
#include <LfQuick/Platform/ScannerSession.h>

#include <QKeyEvent>

namespace LfQuick {

KeyboardWedgeScanner::KeyboardWedgeScanner(QObject *parent)
    : QObject(parent)
    , m_session(new ScannerSession(this))
{
}

bool KeyboardWedgeScanner::isEnabled() const
{
    return m_enabled;
}

void KeyboardWedgeScanner::setEnabled(bool value)
{
    if (m_enabled == value)
        return;
    m_enabled = value;
    if (!m_enabled)
        m_session->reset();
    emit enabledChanged();
}

ScannerSession *KeyboardWedgeScanner::session() const
{
    return m_session;
}

bool KeyboardWedgeScanner::handleKeyEvent(QKeyEvent *event)
{
    if (!m_enabled || !event || event->type() != QEvent::KeyPress || event->isAutoRepeat())
        return false;

    const auto modifiers = event->modifiers();
    if (modifiers.testFlag(Qt::ControlModifier)
        || modifiers.testFlag(Qt::AltModifier)
        || modifiers.testFlag(Qt::MetaModifier)) {
        return false;
    }

    switch (event->key()) {
    case Qt::Key_Return:
    case Qt::Key_Enter:
    case Qt::Key_Tab:
        if (m_session->finish()) {
            event->accept();
            return true;
        }
        return m_session->isActive();
    case Qt::Key_Backspace:
        m_session->reset();
        event->accept();
        return true;
    default:
        break;
    }

    const QString text = event->text();
    if (text.isEmpty() || text.at(0).isSpace())
        return false;

    m_session->feed(text);
    event->accept();
    return true;
}

void KeyboardWedgeScanner::simulateScan(const QString &value)
{
    m_session->simulateScan(value);
}

} // namespace LfQuick
