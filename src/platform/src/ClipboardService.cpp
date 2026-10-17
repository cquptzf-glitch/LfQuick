#include <LfQuick/Platform/ClipboardService.h>

#include <QClipboard>
#include <QCoreApplication>
#include <QGuiApplication>

namespace LfQuick {

ClipboardService::ClipboardService(QObject *parent)
    : QObject(parent)
{
    if (auto *value = clipboard()) {
        connect(value, &QClipboard::dataChanged,
                this, &ClipboardService::textChanged);
    }
}

QString ClipboardService::text() const
{
    if (auto *value = clipboard())
        return value->text(QClipboard::Clipboard);
    return {};
}

void ClipboardService::setText(const QString &textValue)
{
    if (auto *value = clipboard())
        value->setText(textValue, QClipboard::Clipboard);
}

bool ClipboardService::hasText() const
{
    return !text().isEmpty();
}

void ClipboardService::copyText(const QString &textValue)
{
    setText(textValue);
}

void ClipboardService::clear()
{
    if (auto *value = clipboard())
        value->clear(QClipboard::Clipboard);
}

QClipboard *ClipboardService::clipboard() const
{
    auto *app = qobject_cast<QGuiApplication *>(QCoreApplication::instance());
    return app ? app->clipboard() : nullptr;
}

} // namespace LfQuick
