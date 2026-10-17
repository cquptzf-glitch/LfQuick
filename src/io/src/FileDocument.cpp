#include <LfQuick/Io/FileDocument.h>

#include <LfQuick/Io/FileWatcher.h>

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSaveFile>

namespace LfQuick {

FileDocument::FileDocument(QObject *parent)
    : QObject(parent)
    , m_watcher(new FileWatcher(this))
{
    connect(m_watcher, &FileWatcher::fileChanged,
            this, &FileDocument::handleExternalChange);
}

QString FileDocument::normalizedPath(const QString &path)
{
    const QString trimmed = path.trimmed();
    if (trimmed.isEmpty())
        return {};
    return QDir::cleanPath(QFileInfo(trimmed).absoluteFilePath());
}

void FileDocument::setPath(const QString &path)
{
    const QString normalized = normalizedPath(path);
    if (m_path == normalized)
        return;

    m_path = normalized;
    setExternalConflict(false);
    setErrorString({});
    refreshWatch();
    emit pathChanged();
    emit metadataChanged();
}

void FileDocument::setData(const QByteArray &data)
{
    if (m_data == data)
        return;
    m_data = data;
    setDirty(true);
    emit contentChanged();
}

void FileDocument::setText(const QString &text)
{
    setData(text.toUtf8());
}

bool FileDocument::exists() const
{
    return !m_path.isEmpty() && QFileInfo::exists(m_path);
}

qint64 FileDocument::size() const
{
    return exists() ? QFileInfo(m_path).size() : 0;
}

QDateTime FileDocument::lastModified() const
{
    return exists() ? QFileInfo(m_path).lastModified() : QDateTime{};
}

void FileDocument::setWatch(bool watch)
{
    if (m_watch == watch)
        return;
    m_watch = watch;
    refreshWatch();
    emit watchChanged();
}

void FileDocument::setAutoReload(bool autoReload)
{
    if (m_autoReload == autoReload)
        return;
    m_autoReload = autoReload;
    if (m_autoReload && !m_watch)
        setWatch(true);
    emit autoReloadChanged();
}

void FileDocument::setReloadWhenDirty(bool reloadWhenDirty)
{
    if (m_reloadWhenDirty == reloadWhenDirty)
        return;
    m_reloadWhenDirty = reloadWhenDirty;
    emit reloadWhenDirtyChanged();
}

bool FileDocument::load()
{
    if (m_path.isEmpty()) {
        setErrorString(QStringLiteral("File path is empty"));
        return false;
    }

    QFile file(m_path);
    if (!file.open(QIODevice::ReadOnly)) {
        setErrorString(file.errorString());
        emit metadataChanged();
        return false;
    }

    const QByteArray loadedData = file.readAll();
    if (file.error() != QFileDevice::NoError) {
        setErrorString(file.errorString());
        return false;
    }

    const bool changed = m_data != loadedData;
    m_data = loadedData;
    setDirty(false);
    setExternalConflict(false);
    setErrorString({});
    refreshWatch();
    if (changed)
        emit contentChanged();
    emit metadataChanged();
    emit loaded();
    return true;
}

bool FileDocument::save()
{
    if (m_path.isEmpty()) {
        setErrorString(QStringLiteral("File path is empty"));
        return false;
    }

    const QFileInfo info(m_path);
    if (!QDir().mkpath(info.absolutePath())) {
        setErrorString(QStringLiteral("Failed to create parent directory"));
        return false;
    }

    QSaveFile file(m_path);
    if (!file.open(QIODevice::WriteOnly)) {
        setErrorString(file.errorString());
        return false;
    }
    if (file.write(m_data) != m_data.size()) {
        setErrorString(file.errorString().isEmpty()
                           ? QStringLiteral("Failed to write complete file")
                           : file.errorString());
        file.cancelWriting();
        return false;
    }
    if (!file.commit()) {
        setErrorString(file.errorString());
        return false;
    }

    setDirty(false);
    setExternalConflict(false);
    setErrorString({});
    refreshWatch();
    emit metadataChanged();
    emit saved();
    return true;
}

bool FileDocument::saveAs(const QString &path)
{
    const QString previous = m_path;
    setPath(path);
    if (m_path.isEmpty()) {
        if (previous != m_path)
            setPath(previous);
        return false;
    }
    return save();
}

void FileDocument::clear()
{
    const bool hadContent = !m_data.isEmpty();
    m_data.clear();
    setDirty(true);
    setErrorString({});
    if (hadContent)
        emit contentChanged();
}

void FileDocument::markDirty()
{
    setDirty(true);
}

void FileDocument::setDirty(bool dirty)
{
    if (m_dirty == dirty)
        return;
    m_dirty = dirty;
    emit dirtyChanged();
}

void FileDocument::setExternalConflict(bool conflict)
{
    if (m_externalConflict == conflict)
        return;
    m_externalConflict = conflict;
    emit externalConflictChanged();
}

void FileDocument::setErrorString(const QString &error)
{
    if (m_errorString == error)
        return;
    m_errorString = error;
    emit errorStringChanged();
}

void FileDocument::refreshWatch()
{
    if (!m_watcher)
        return;
    m_watcher->clear();
    if (m_watch && !m_path.isEmpty())
        m_watcher->watchFile(m_path);
}

void FileDocument::handleExternalChange(const QString &path)
{
    if (m_path.isEmpty() || normalizedPath(path) != m_path)
        return;

    emit metadataChanged();
    emit externalChanged(m_path);

    if (m_dirty && !m_reloadWhenDirty) {
        setExternalConflict(true);
        emit externalConflictDetected(m_path);
        return;
    }

    if (m_autoReload)
        load();
}

} // namespace LfQuick
