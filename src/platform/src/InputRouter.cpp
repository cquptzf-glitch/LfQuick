#include <LfQuick/Platform/InputRouter.h>
#include <LfQuick/Platform/KeyboardWedgeScanner.h>

#include <QEvent>
#include <QKeyEvent>

namespace LfQuick {

InputRouter::InputRouter(QObject *parent)
    : QObject(parent)
{
}

InputRouter::~InputRouter()
{
    uninstall();
}

KeyboardWedgeScanner *InputRouter::scanner() const
{
    return m_scanner;
}

void InputRouter::setScanner(KeyboardWedgeScanner *scanner)
{
    if (m_scanner == scanner)
        return;
    m_scanner = scanner;
    emit scannerChanged();
}

bool InputRouter::installed() const
{
    return !m_target.isNull();
}

bool InputRouter::installOn(QObject *target)
{
    if (!target)
        return false;

    if (m_target == target)
        return true;

    uninstall();
    m_target = target;
    m_target->installEventFilter(this);
    emit installedChanged();
    return true;
}

void InputRouter::uninstall()
{
    if (m_target) {
        m_target->removeEventFilter(this);
        m_target.clear();
        emit installedChanged();
    }
}

bool InputRouter::eventFilter(QObject *watched, QEvent *event)
{
    if (!m_scanner || !m_scanner->isEnabled() || event->type() != QEvent::KeyPress)
        return QObject::eventFilter(watched, event);

    if (m_scanner->handleKeyEvent(static_cast<QKeyEvent *>(event)))
        return true;

    return QObject::eventFilter(watched, event);
}

} // namespace LfQuick
