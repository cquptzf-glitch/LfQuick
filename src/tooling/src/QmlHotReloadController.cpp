#include <LfQuick/Tooling/QmlHotReloadController.h>

#include <QDir>
#include <QFileInfo>
#include <QFileSystemWatcher>
#include <QQmlEngine>
#include <QTimer>
#include <QtQml/qqml.h>

#include <algorithm>

namespace LfQuick {

QmlHotReloadController::QmlHotReloadController(QObject *parent)
    : QObject(parent)
    , m_watcher(new QFileSystemWatcher(this))
    , m_debounceTimer(new QTimer(this))
{
    m_debounceTimer->setSingleShot(true);
    m_debounceTimer->setInterval(m_debounceMs);

    connect(m_watcher, &QFileSystemWatcher::fileChanged,
            this, [this](const QString &path) {
                queuePath(path);
                rebuildWatcher();
            });
    connect(m_watcher, &QFileSystemWatcher::directoryChanged,
            this, [this](const QString &path) {
                queuePath(path);
                rebuildWatcher();
            });
    connect(m_debounceTimer, &QTimer::timeout,
            this, &QmlHotReloadController::flushPending);
}

void QmlHotReloadController::setEnabled(bool value)
{
    if (m_enabled == value)
        return;

    m_enabled = value;
    if (!m_enabled) {
        m_debounceTimer->stop();
        m_pendingLogicalSources.clear();
    } else {
        rebuildWatcher();
    }
    emit enabledChanged();
}

void QmlHotReloadController::setDebounceMs(int value)
{
    value = qBound(0, value, 5000);
    if (m_debounceMs == value)
        return;

    m_debounceMs = value;
    m_debounceTimer->setInterval(m_debounceMs);
    emit debounceMsChanged();
}

void QmlHotReloadController::setEngine(QQmlEngine *engine)
{
    if (m_engine == engine)
        return;
    m_engine = engine;
    setLastError({});
}

bool QmlHotReloadController::watchSource(const QUrl &logicalSource,
                                         const QUrl &developmentSource)
{
    if (!logicalSource.isValid() || logicalSource.isEmpty()) {
        setLastError(QStringLiteral("Hot reload logical source is empty or invalid"));
        return false;
    }

    const QUrl effectiveSource = developmentSource.isEmpty()
        ? logicalSource
        : developmentSource;
    if (!effectiveSource.isLocalFile()) {
        setLastError(QStringLiteral("Hot reload development source must be a local file URL: %1")
                         .arg(effectiveSource.toString()));
        return false;
    }

    const QString logicalKey = logicalSource.toString(QUrl::FullyEncoded);
    const QFileInfo info(effectiveSource.toLocalFile());
    const QString filePath = QDir::cleanPath(info.absoluteFilePath());
    if (filePath.isEmpty()) {
        setLastError(QStringLiteral("Hot reload development source has no local path"));
        return false;
    }

    const auto existing = m_logicalToFile.constFind(logicalKey);
    if (existing != m_logicalToFile.cend()) {
        if (QDir::cleanPath(existing.value()) != filePath) {
            setLastError(QStringLiteral("Hot reload source '%1' is already mapped to '%2'; "
                                        "release all owners before remapping to '%3'")
                             .arg(logicalSource.toString(), existing.value(), filePath));
            return false;
        }
        m_watchRefCounts[logicalKey] = m_watchRefCounts.value(logicalKey, 1) + 1;
        setLastError({});
        return true;
    }

    m_logicalToFile.insert(logicalKey, filePath);
    m_watchRefCounts.insert(logicalKey, 1);
    if (!developmentSource.isEmpty())
        m_overrides.insert(logicalKey, QUrl::fromLocalFile(filePath));
    else
        m_overrides.remove(logicalKey);

    rebuildWatcher();
    setLastError({});
    emit watchedSourcesChanged();
    return true;
}

bool QmlHotReloadController::unwatchSource(const QUrl &logicalSource)
{
    const QString logicalKey = logicalSource.toString(QUrl::FullyEncoded);
    const auto refIt = m_watchRefCounts.find(logicalKey);
    if (refIt == m_watchRefCounts.end())
        return false;

    if (refIt.value() > 1) {
        --refIt.value();
        return true;
    }

    m_watchRefCounts.erase(refIt);
    const bool removed = m_logicalToFile.remove(logicalKey) > 0;
    m_overrides.remove(logicalKey);
    m_pendingLogicalSources.remove(logicalKey);
    if (removed) {
        rebuildWatcher();
        emit watchedSourcesChanged();
    }
    return removed;
}

void QmlHotReloadController::clearWatches()
{
    if (m_logicalToFile.isEmpty() && m_overrides.isEmpty() && m_watchRefCounts.isEmpty())
        return;

    m_logicalToFile.clear();
    m_overrides.clear();
    m_watchRefCounts.clear();
    m_pendingLogicalSources.clear();
    m_debounceTimer->stop();
    rebuildWatcher();
    emit watchedSourcesChanged();
}

QUrl QmlHotReloadController::resolveSource(const QUrl &logicalSource) const
{
    const QString logicalKey = logicalSource.toString(QUrl::FullyEncoded);
    const auto it = m_overrides.constFind(logicalKey);
    return it == m_overrides.cend() ? logicalSource : it.value();
}

void QmlHotReloadController::requestReload(const QUrl &logicalSource)
{
    if (!m_enabled)
        return;

    if (logicalSource.isEmpty()) {
        for (auto it = m_logicalToFile.cbegin(); it != m_logicalToFile.cend(); ++it)
            m_pendingLogicalSources.insert(it.key());
    } else {
        m_pendingLogicalSources.insert(logicalSource.toString(QUrl::FullyEncoded));
    }

    if (m_debounceMs == 0)
        flushPending();
    else
        m_debounceTimer->start();
}

void QmlHotReloadController::rebuildWatcher()
{
    const QStringList existingFiles = m_watcher->files();
    if (!existingFiles.isEmpty())
        m_watcher->removePaths(existingFiles);
    const QStringList existingDirectories = m_watcher->directories();
    if (!existingDirectories.isEmpty())
        m_watcher->removePaths(existingDirectories);

    if (!m_enabled)
        return;

    QSet<QString> files;
    QSet<QString> directories;
    for (auto it = m_logicalToFile.cbegin(); it != m_logicalToFile.cend(); ++it) {
        const QFileInfo info(it.value());
        const QString directory = info.absolutePath();
        if (QFileInfo::exists(directory))
            directories.insert(directory);
        if (info.exists() && info.isFile())
            files.insert(info.absoluteFilePath());
    }

    if (!files.isEmpty())
        m_watcher->addPaths(files.values());
    if (!directories.isEmpty())
        m_watcher->addPaths(directories.values());
}

void QmlHotReloadController::queuePath(const QString &path)
{
    if (!m_enabled)
        return;

    const QString cleanPath = QDir::cleanPath(path);
    for (auto it = m_logicalToFile.cbegin(); it != m_logicalToFile.cend(); ++it) {
        const QFileInfo sourceInfo(it.value());
        if (QDir::cleanPath(sourceInfo.absoluteFilePath()) == cleanPath
            || QDir::cleanPath(sourceInfo.absolutePath()) == cleanPath) {
            m_pendingLogicalSources.insert(it.key());
        }
    }

    if (!m_pendingLogicalSources.isEmpty()) {
        if (m_debounceMs == 0)
            flushPending();
        else
            m_debounceTimer->start();
    }
}

void QmlHotReloadController::flushPending()
{
    if (!m_enabled || m_pendingLogicalSources.isEmpty())
        return;

    QQmlEngine *qmlEngine = effectiveEngine();
    if (!qmlEngine) {
        setLastError(QStringLiteral("Hot reload requires a QQmlEngine"));
        m_pendingLogicalSources.clear();
        return;
    }

    rebuildWatcher();
    qmlEngine->clearComponentCache();

    QStringList sources = m_pendingLogicalSources.values();
    std::sort(sources.begin(), sources.end());
    m_pendingLogicalSources.clear();
    ++m_revision;
    setLastError({});
    emit revisionChanged();
    emit reloadRequested(sources, m_revision);
}

void QmlHotReloadController::setLastError(const QString &error)
{
    if (m_lastError == error)
        return;
    m_lastError = error;
    emit lastErrorChanged();
}

QQmlEngine *QmlHotReloadController::effectiveEngine() const
{
    if (m_engine)
        return m_engine;
    return qmlEngine(const_cast<QmlHotReloadController *>(this));
}

} // namespace LfQuick
