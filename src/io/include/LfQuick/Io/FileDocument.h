#pragma once

#include <LfQuick/Io/LfQuickIoGlobal.h>

#include <QByteArray>
#include <QDateTime>
#include <QObject>
#include <QString>
#include <QtQmlIntegration/qqmlintegration.h>

namespace LfQuick {

class FileWatcher;

class LFQUICKIO_EXPORT FileDocument : public QObject
{
    Q_OBJECT
    QML_NAMED_ELEMENT(LfFileDocument)
    Q_PROPERTY(QString path READ path WRITE setPath NOTIFY pathChanged FINAL)
    Q_PROPERTY(QByteArray data READ data WRITE setData NOTIFY contentChanged FINAL)
    Q_PROPERTY(QString text READ text WRITE setText NOTIFY contentChanged FINAL)
    Q_PROPERTY(bool exists READ exists NOTIFY metadataChanged FINAL)
    Q_PROPERTY(qint64 size READ size NOTIFY metadataChanged FINAL)
    Q_PROPERTY(QDateTime lastModified READ lastModified NOTIFY metadataChanged FINAL)
    Q_PROPERTY(bool dirty READ dirty NOTIFY dirtyChanged FINAL)
    Q_PROPERTY(bool watch READ watch WRITE setWatch NOTIFY watchChanged FINAL)
    Q_PROPERTY(bool autoReload READ autoReload WRITE setAutoReload NOTIFY autoReloadChanged FINAL)
    Q_PROPERTY(bool reloadWhenDirty READ reloadWhenDirty WRITE setReloadWhenDirty NOTIFY reloadWhenDirtyChanged FINAL)
    Q_PROPERTY(bool externalConflict READ externalConflict NOTIFY externalConflictChanged FINAL)
    Q_PROPERTY(QString errorString READ errorString NOTIFY errorStringChanged FINAL)

public:
    explicit FileDocument(QObject *parent = nullptr);

    QString path() const { return m_path; }
    void setPath(const QString &path);

    QByteArray data() const { return m_data; }
    void setData(const QByteArray &data);
    QString text() const { return QString::fromUtf8(m_data); }
    void setText(const QString &text);

    bool exists() const;
    qint64 size() const;
    QDateTime lastModified() const;
    bool dirty() const noexcept { return m_dirty; }

    bool watch() const noexcept { return m_watch; }
    void setWatch(bool watch);
    bool autoReload() const noexcept { return m_autoReload; }
    void setAutoReload(bool autoReload);
    bool reloadWhenDirty() const noexcept { return m_reloadWhenDirty; }
    void setReloadWhenDirty(bool reloadWhenDirty);
    bool externalConflict() const noexcept { return m_externalConflict; }

    QString errorString() const { return m_errorString; }

    Q_INVOKABLE bool load();
    Q_INVOKABLE bool reload() { return load(); }
    Q_INVOKABLE bool save();
    Q_INVOKABLE bool saveAs(const QString &path);
    Q_INVOKABLE void clear();
    Q_INVOKABLE void markDirty();

signals:
    void pathChanged();
    void contentChanged();
    void metadataChanged();
    void dirtyChanged();
    void watchChanged();
    void autoReloadChanged();
    void reloadWhenDirtyChanged();
    void externalConflictChanged();
    void errorStringChanged();
    void loaded();
    void saved();
    void externalChanged(const QString &path);
    void externalConflictDetected(const QString &path);

private:
    static QString normalizedPath(const QString &path);
    bool writeDataToPath(const QString &path);
    void setDirty(bool dirty);
    void setExternalConflict(bool conflict);
    void setErrorString(const QString &error);
    void refreshWatch();
    void handleExternalChange(const QString &path);
    void armSelfWriteSuppression(const QString &path, const QByteArray &data);
    void clearSelfWriteSuppression();
    bool matchesSuppressedSelfWrite(const QString &path);

    QString m_path;
    QByteArray m_data;
    bool m_dirty = false;
    bool m_watch = false;
    bool m_autoReload = false;
    bool m_reloadWhenDirty = false;
    bool m_externalConflict = false;
    QString m_errorString;
    FileWatcher *m_watcher = nullptr;
    QString m_selfWritePath;
    QByteArray m_selfWriteData;
    quint64 m_selfWriteGeneration = 0;
    bool m_selfWriteSuppressionActive = false;
};

} // namespace LfQuick
