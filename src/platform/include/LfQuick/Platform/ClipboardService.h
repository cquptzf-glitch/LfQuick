#pragma once

#include <LfQuick/Platform/LfQuickPlatformGlobal.h>

#include <QObject>
#include <QString>

class QClipboard;

namespace LfQuick {

class LFQUICKPLATFORM_EXPORT ClipboardService : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString text READ text WRITE setText NOTIFY textChanged FINAL)
    Q_PROPERTY(bool hasText READ hasText NOTIFY textChanged FINAL)

public:
    explicit ClipboardService(QObject *parent = nullptr);

    QString text() const;
    void setText(const QString &text);
    bool hasText() const;

    Q_INVOKABLE void copyText(const QString &text);
    Q_INVOKABLE void clear();

signals:
    void textChanged();

private:
    QClipboard *clipboard() const;
};

} // namespace LfQuick
