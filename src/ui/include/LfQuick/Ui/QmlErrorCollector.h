#pragma once

#include <LfQuick/Ui/LfQuickUiGlobal.h>

#include <QObject>
#include <QUrl>
#include <QVariantList>
#include <QVariantMap>
#include <QtQml/qqmlintegration.h>

namespace LfQuick {

class LFQUICKUI_EXPORT QmlErrorCollector : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(int count READ count NOTIFY changed FINAL)
    Q_PROPERTY(QVariantList entries READ entries NOTIFY changed FINAL)
    Q_PROPERTY(QVariantMap lastError READ lastError NOTIFY changed FINAL)

public:
    explicit QmlErrorCollector(QObject *parent = nullptr);

    int count() const noexcept;
    QVariantList entries() const;
    QVariantMap lastError() const;

    void record(const QString &route,
                const QUrl &source,
                const QString &message,
                const QString &stage = QStringLiteral("load"));

    Q_INVOKABLE void clear();

signals:
    void changed();
    void errorRecorded(const QVariantMap &entry);

private:
    QVariantList m_entries;
};

} // namespace LfQuick
