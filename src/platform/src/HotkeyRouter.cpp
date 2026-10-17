#include <LfQuick/Platform/HotkeyRouter.h>

#include <QEvent>
#include <QKeyEvent>
#include <QVariantMap>

#include <utility>

namespace LfQuick {

HotkeyRouter::HotkeyRouter(QObject *parent)
    : QObject(parent)
{
}

HotkeyRouter::~HotkeyRouter()
{
    uninstall();
}

bool HotkeyRouter::installOn(QObject *target)
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

void HotkeyRouter::uninstall()
{
    if (!m_target)
        return;
    m_target->removeEventFilter(this);
    m_target.clear();
    emit installedChanged();
}

bool HotkeyRouter::registerHotkey(const QString &id,
                                  const QString &sequence,
                                  bool enabled,
                                  bool allowAutoRepeat)
{
    const QString normalizedId = id.trimmed();
    const QKeySequence parsed(sequence, QKeySequence::PortableText);
    if (normalizedId.isEmpty() || parsed.isEmpty())
        return false;

    Entry entry;
    entry.id = normalizedId;
    entry.sequence = parsed;
    entry.enabled = enabled;
    entry.allowAutoRepeat = allowAutoRepeat;
    m_hotkeys.insert(normalizedId, entry);
    emit hotkeysChanged();
    return true;
}

bool HotkeyRouter::unregisterHotkey(const QString &id)
{
    const bool removed = m_hotkeys.remove(id.trimmed());
    if (removed)
        emit hotkeysChanged();
    return removed;
}

bool HotkeyRouter::setHotkeyEnabled(const QString &id, bool enabled)
{
    auto it = m_hotkeys.find(id.trimmed());
    if (it == m_hotkeys.end())
        return false;
    if (it->enabled == enabled)
        return true;
    it->enabled = enabled;
    emit hotkeysChanged();
    return true;
}

void HotkeyRouter::clear()
{
    if (m_hotkeys.isEmpty())
        return;
    m_hotkeys.clear();
    emit hotkeysChanged();
}

QVariantList HotkeyRouter::hotkeys() const
{
    QVariantList result;
    result.reserve(m_hotkeys.size());
    for (const Entry &entry : std::as_const(m_hotkeys)) {
        result.push_back(QVariantMap{
            {QStringLiteral("id"), entry.id},
            {QStringLiteral("sequence"), entry.sequence.toString(QKeySequence::PortableText)},
            {QStringLiteral("enabled"), entry.enabled},
            {QStringLiteral("allowAutoRepeat"), entry.allowAutoRepeat}
        });
    }
    return result;
}

bool HotkeyRouter::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() != QEvent::KeyPress)
        return QObject::eventFilter(watched, event);

    auto *keyEvent = static_cast<QKeyEvent *>(event);
    const QKeySequence pressed(keyEvent->keyCombination());

    for (const Entry &entry : std::as_const(m_hotkeys)) {
        if (!entry.enabled)
            continue;
        if (keyEvent->isAutoRepeat() && !entry.allowAutoRepeat)
            continue;
        if (entry.sequence.matches(pressed) != QKeySequence::ExactMatch)
            continue;

        emit triggered(entry.id);
        keyEvent->accept();
        return true;
    }

    return QObject::eventFilter(watched, event);
}

} // namespace LfQuick
