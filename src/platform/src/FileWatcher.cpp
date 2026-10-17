#include <LfQuick/Platform/FileWatcher.h>

#include <QDir>
#include <QFileInfo>
#include <QFileSystemWatcher>
#include <QTimer>

#include <algorithm>
#include <utility>

namespace LfQuick {

FileWatcher::FileWatcher(QObject *parent)
    : QObject(parent)
    , m_watcher(new QFileSystemWatcher(this))
{
    connect(m_watcher, &QFileSystemWatcher::fileChanged,
            this, [this](const QString &path) {
                emit fileChanged(QDir::cleanPath(path));
                scheduleRebuild();
            });

    connect(m_watcher, &QFileSystemWatcher::directoryChanged,
            this, [this](const QString &path) {
                const QString normalized = QDir::cleanPath(path);
                if (m_directories.contains(normalized))
                    emit directoryChanged(normalized);

                for (const QString &file : std::as_const(m_files)) {
                    if (QFileInfo(file).absolutePath() == normalized && QFileInfo::exists(file))
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

    if (m_files.contains(normalized))
        return true;

    m_files.insert(normalized);
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

    const bool removed = m_files.remove(normalized) || m_directories.remove(normalized);
    if (!removed)
        return false;

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

void FileWatcher::rebuildWatcher()
{
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
    QTimer::singleShot(30, this, &FileWatcher::rebuildWatcher);
}

} // namespace LfQuick
