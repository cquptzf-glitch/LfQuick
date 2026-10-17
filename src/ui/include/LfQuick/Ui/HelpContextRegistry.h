#pragma once

#include <LfQuick/Ui/LfQuickUiGlobal.h>

#include <QObject>
#include <QStringList>
#include <QVariantMap>
#include <QtQmlIntegration/qqmlintegration.h>

namespace LfQuick {

class LFQUICKUI_EXPORT HelpContextRegistry : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(int count READ count NOTIFY contextsChanged FINAL)
    Q_PROPERTY(QVariantMap contexts READ contexts WRITE setContexts NOTIFY contextsChanged FINAL)

public:
    explicit HelpContextRegistry(QObject *parent = nullptr);

    int count() const noexcept { return m_contexts.size(); }
    QVariantMap contexts() const { return m_contexts; }
    void setContexts(const QVariantMap &contexts);

    Q_INVOKABLE bool registerContext(const QString &id, const QVariantMap &entry);
    Q_INVOKABLE bool unregisterContext(const QString &id);
    Q_INVOKABLE bool contains(const QString &id) const;
    Q_INVOKABLE QVariantMap resolve(const QString &id) const;
    Q_INVOKABLE QString title(const QString &id) const;
    Q_INVOKABLE QString text(const QString &id) const;
    Q_INVOKABLE QString url(const QString &id) const;
    Q_INVOKABLE QStringList ids() const;
    Q_INVOKABLE void clear();

signals:
    void contextsChanged();
    void helpRequested(const QString &id, const QVariantMap &entry);

public slots:
    bool requestHelp(const QString &id);

private:
    static QString normalizedId(const QString &id);
    static QVariantMap normalizedEntry(const QVariantMap &entry);

    QVariantMap m_contexts;
};

} // namespace LfQuick
