#include <LfQuick/Io/FileWatcher.h>

#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QFileSystemWatcher>
#include <QTimer>

#include <algorithm>
#include <utility>

namespace LfQuick {
namespace {

qint64 timestampMs(const QDateTime &value)
{
    return value.isValid() ? value.toMSecsSinceEpoch() : -1;
}

} // namespace

FileWatcher::FileWatcher(QObject *parent)
    : QObject(parent)
    , m_watcher(new QFileSystemWatcher(this))
    , m_rebuildTimer(new QTimer(this))
{
    m_rebuildTimer->setSingleShot(true);
    m_rebuildTimer->setInterval(30);
    connect(m_rebuildTimer, &QTimer::timeout,
            this, &FileWatcher::rebuildWatcher);

    connect(m_watcher, &QFileSystemWatcher::fileChanged,
            this, [this](const QString &path) {
                const QString normalized = normalizedPath(path);
                if (!m_files.contains(normalized))
                    return;
                updateFileState(normalized);
                emit fileChanged(normalized);
                scheduleRebuild();
            });

    connect(m_watcher, &QFileSystemWatcher::directoryChanged,
            this, [this](const QString &path) {
                const QString normalized = normalizedPath(path);
                if (m_directories.contains(normalized))
                    emit directoryChanged(normalized);

                // QFileSystemWatcher can drop a direct file watch after an
                // atomic replace/rename. The parent-directory watch keeps the
                // logical file watch alive, but unrelated directory activity
                // must not be reported as a change to every watched file.
                for (const QString &file : std::as_const(m_files)) {
                    if (QFileInfo(file).absolutePath() != normalized)
                        continue;
                    if (updateFileState(file))
                        emit fileChanged(file);
                }
                scheduleRebuild();
            });
}

QStringList FileWatcher::watchedFiles() const
{
    QStringList result = m_files.values();
    std::sort(result.begin(), result.end());
    return result;
}

QStringList FileWatcher::watchedDirectories() const
{
    QStringList result = m_directories.values();
    std::sort(result.begin(), result.end());
    return result;
}

int FileWatcher::count() const noexcept
{
    return m_files.size() + m_directories.size();
}

bool FileWatcher::watchFile(const QString &path)
{
    const QString normalized = normalizedPath(path);
    if (normalized.isEmpty())
        return false;

    const QFileInfo info(normalized);
    if (!QFileInfo::exists(info.absolutePath()))
        return false;
    if (info.exists() && !info.isFile())
        return false;

    if (m_files.contains(normalized))
        return true;

    m_directories.remove(normalized);
    m_files.insert(normalized);
    m_fileStates.insert(normalized, snapshotFile(normalized));
    rebuildWatcher();
    emit watchesChanged();
    return true;
}

bool FileWatcher::watchDirectory(const QString &path)
{
    const QString normalized = normalizedPath(path);
    if (normalized.isEmpty())
        return false;

    const QFileInfo info(normalized);
    if (!info.exists() || !info.isDir())
        return false;

    if (m_directories.contains(normalized))
        return true;

    m_files.remove(normalized);
    m_fileStates.remove(normalized);
    m_directories.insert(normalized);
    rebuildWatcher();
    emit watchesChanged();
    return true;
}

bool FileWatcher::unwatch(const QString &path)
{
    const QString normalized = normalizedPath(path);
    if (normalized.isEmpty())
        return false;

    const bool removedFile = m_files.remove(normalized) > 0;
    const bool removedDirectory = m_directories.remove(normalized) > 0;
    if (!removedFile && !removedDirectory)
        return false;

    if (removedFile)
        m_fileStates.remove(normalized);
    rebuildWatcher();
    emit watchesChanged();
    return true;
}

bool FileWatcher::isWatching(const QString &path) const
{
    const QString normalized = normalizedPath(path);
    return !normalized.isEmpty()
        && (m_files.contains(normalized) || m_directories.contains(normalized));
}

void FileWatcher::clear()
{
    if (m_files.isEmpty() && m_directories.isEmpty())
        return;

    m_files.clear();
    m_directories.clear();
    m_fileStates.clear();
    rebuildWatcher();
    emit watchesChanged();
}

QString FileWatcher::normalizedPath(const QString &path)
{
    const QString trimmed = path.trimmed();
    if (trimmed.isEmpty())
        return {};
    return QDir::cleanPath(QFileInfo(trimmed).absoluteFilePath());
}

FileWatcher::FileState FileWatcher::snapshotFile(const QString &path)
{
    QFileInfo info(path);
    info.setCaching(false);

    FileState state;
    state.exists = info.exists();
    state.file = state.exists && info.isFile();
    if (state.file) {
        state.size = info.size();
        state.lastModifiedMs = timestampMs(info.lastModified());
        state.metadataChangedMs = timestampMs(info.metadataChangeTime());
    }
    return state;
}

bool FileWatcher::updateFileState(const QString &path)
{
    const FileState current = snapshotFile(path);
    auto it = m_fileStates.find(path);
    if (it == m_fileStates.end()) {
        m_fileStates.insert(path, current);
        return false;
    }

    const bool changed = it.value() != current;
    it.value() = current;
    return changed;
}

void FileWatcher::rebuildWatcher()
{
    if (m_rebuildTimer)
        m_rebuildTimer->stop();

    const QStringList existingFiles = m_watcher->files();
    if (!existingFiles.isEmpty())
        m_watcher->removePaths(existingFiles);

    const QStringList existingDirectories = m_watcher->directories();
    if (!existingDirectories.isEmpty())
        m_watcher->removePaths(existingDirectories);

    QSet<QString> filesToWatch;
    QSet<QString> directoriesToWatch = m_directories;

    for (const QString &file : std::as_const(m_files)) {
        const QFileInfo info(file);
        directoriesToWatch.insert(info.absolutePath());
        if (info.exists() && info.isFile())
            filesToWatch.insert(info.absoluteFilePath());
    }

    QStringList existingDirectoryPaths;
    for (const QString &directory : std::as_const(directoriesToWatch)) {
        if (QFileInfo(directory).isDir())
            existingDirectoryPaths.push_back(directory);
    }

    if (!filesToWatch.isEmpty())
        m_watcher->addPaths(filesToWatch.values());
    if (!existingDirectoryPaths.isEmpty())
        m_watcher->addPaths(existingDirectoryPaths);
}

void FileWatcher::scheduleRebuild()
{
    if (m_rebuildTimer)
        m_rebuildTimer->start();
}

} // namespace LfQuick
