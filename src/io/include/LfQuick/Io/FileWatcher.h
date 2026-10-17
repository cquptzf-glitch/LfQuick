#pragma once

#include <LfQuick/Io/LfQuickIoGlobal.h>

#include <QHash>
#include <QObject>
#include <QSet>
#include <QStringList>
#include <QtQmlIntegration/qqmlintegration.h>

class QFileSystemWatcher;
class QTimer;

namespace LfQuick {

class LFQUICKIO_EXPORT FileWatcher : public QObject
{
    Q_OBJECT
    QML_NAMED_ELEMENT(LfFileWatcher)
    Q_PROPERTY(QStringList watchedFiles READ watchedFiles NOTIFY watchesChanged FINAL)
    Q_PROPERTY(QStringList watchedDirectories READ watchedDirectories NOTIFY watchesChanged FINAL)
    Q_PROPERTY(int count READ count NOTIFY watchesChanged FINAL)

public:
    explicit FileWatcher(QObject *parent = nullptr);

    QStringList watchedFiles() const;
    QStringList watchedDirectories() const;
    int count() const noexcept;

    Q_INVOKABLE bool watchFile(const QString &path);
    Q_INVOKABLE bool watchDirectory(const QString &path);
    Q_INVOKABLE bool unwatch(const QString &path);
    Q_INVOKABLE bool isWatching(const QString &path) const;
    Q_INVOKABLE void clear();

signals:
    void watchesChanged();
    void fileChanged(const QString &path);
    void directoryChanged(const QString &path);

private:
    struct FileState {
        bool exists = false;
        bool file = false;
        qint64 size = -1;
        qint64 lastModifiedMs = -1;
        qint64 metadataChangedMs = -1;

        bool operator==(const FileState &) const = default;
    };

    static QString normalizedPath(const QString &path);
    static FileState snapshotFile(const QString &path);
    bool updateFileState(const QString &path);
    void rebuildWatcher();
    void scheduleRebuild();

    QFileSystemWatcher *m_watcher = nullptr;
    QTimer *m_rebuildTimer = nullptr;
    QSet<QString> m_files;
    QSet<QString> m_directories;
    QHash<QString, FileState> m_fileStates;
};

} // namespace LfQuick
