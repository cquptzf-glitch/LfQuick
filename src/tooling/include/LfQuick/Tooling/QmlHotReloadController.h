#pragma once

#include <LfQuick/Tooling/LfQuickToolingGlobal.h>

#include <QHash>
#include <QObject>
#include <QPointer>
#include <QSet>
#include <QStringList>
#include <QUrl>
#include <QtQmlIntegration/qqmlintegration.h>

class QFileSystemWatcher;
class QQmlEngine;
class QTimer;

namespace LfQuick {

class LFQUICKTOOLING_EXPORT QmlHotReloadController : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(bool enabled READ isEnabled WRITE setEnabled NOTIFY enabledChanged FINAL)
    Q_PROPERTY(int debounceMs READ debounceMs WRITE setDebounceMs NOTIFY debounceMsChanged FINAL)
    Q_PROPERTY(qint64 revision READ revision NOTIFY revisionChanged FINAL)
    Q_PROPERTY(int watchedSourceCount READ watchedSourceCount NOTIFY watchedSourcesChanged FINAL)
    Q_PROPERTY(QString lastError READ lastError NOTIFY lastErrorChanged FINAL)

public:
    explicit QmlHotReloadController(QObject *parent = nullptr);

    bool isEnabled() const noexcept { return m_enabled; }
    void setEnabled(bool value);

    int debounceMs() const noexcept { return m_debounceMs; }
    void setDebounceMs(int value);

    qint64 revision() const noexcept { return m_revision; }
    int watchedSourceCount() const noexcept { return m_logicalToFile.size(); }
    QString lastError() const { return m_lastError; }

    QQmlEngine *engine() const noexcept { return m_engine; }
    void setEngine(QQmlEngine *engine);

    // watchSource()/unwatchSource() form a shared ownership pair. Repeated
    // acquisitions of the same logical -> development mapping are reference
    // counted so multiple hot-reload hosts can safely share one source.
    Q_INVOKABLE bool watchSource(const QUrl &logicalSource,
                                 const QUrl &developmentSource = {});
    Q_INVOKABLE bool unwatchSource(const QUrl &logicalSource);
    Q_INVOKABLE void clearWatches();
    Q_INVOKABLE QUrl resolveSource(const QUrl &logicalSource) const;
    Q_INVOKABLE void requestReload(const QUrl &logicalSource = {});

signals:
    void enabledChanged();
    void debounceMsChanged();
    void revisionChanged();
    void watchedSourcesChanged();
    void lastErrorChanged();
    void reloadRequested(const QStringList &logicalSources, qint64 revision);

private:
    void rebuildWatcher();
    void queuePath(const QString &path);
    void flushPending();
    void setLastError(const QString &error);
    QQmlEngine *effectiveEngine() const;

    bool m_enabled = true;
    int m_debounceMs = 120;
    qint64 m_revision = 0;
    QString m_lastError;
    QPointer<QQmlEngine> m_engine;
    QFileSystemWatcher *m_watcher = nullptr;
    QTimer *m_debounceTimer = nullptr;
    QHash<QString, QUrl> m_overrides;
    QHash<QString, QString> m_logicalToFile;
    QHash<QString, int> m_watchRefCounts;
    QSet<QString> m_pendingLogicalSources;
};

} // namespace LfQuick
