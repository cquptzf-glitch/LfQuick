#pragma once

#include <LfQuick/Io/FileDocument.h>
#include <LfQuick/Io/LfQuickIoGlobal.h>

#include <QObject>
#include <QVariant>
#include <QtQmlIntegration/qqmlintegration.h>

namespace LfQuick {

class LFQUICKIO_EXPORT JsonDocument : public QObject
{
    Q_OBJECT
    QML_NAMED_ELEMENT(LfJsonDocument)
    Q_PROPERTY(QString path READ path WRITE setPath NOTIFY pathChanged FINAL)
    Q_PROPERTY(QVariant value READ value WRITE setValue NOTIFY valueChanged FINAL)
    Q_PROPERTY(bool dirty READ dirty NOTIFY dirtyChanged FINAL)
    Q_PROPERTY(bool watch READ watch WRITE setWatch NOTIFY watchChanged FINAL)
    Q_PROPERTY(bool autoReload READ autoReload WRITE setAutoReload NOTIFY autoReloadChanged FINAL)
    Q_PROPERTY(bool compact READ compact WRITE setCompact NOTIFY compactChanged FINAL)
    Q_PROPERTY(QString errorString READ errorString NOTIFY errorStringChanged FINAL)
    Q_PROPERTY(FileDocument* fileDocument READ fileDocument CONSTANT FINAL)

public:
    explicit JsonDocument(QObject *parent = nullptr);

    QString path() const;
    void setPath(const QString &path);

    QVariant value() const { return m_value; }
    void setValue(const QVariant &value);

    bool dirty() const noexcept { return m_dirty; }
    bool watch() const;
    void setWatch(bool watch);
    bool autoReload() const;
    void setAutoReload(bool autoReload);

    bool compact() const noexcept { return m_compact; }
    void setCompact(bool compact);

    QString errorString() const { return m_errorString; }
    FileDocument *fileDocument() const { return m_file; }

    Q_INVOKABLE bool load();
    Q_INVOKABLE bool reload() { return load(); }
    Q_INVOKABLE bool save();
    Q_INVOKABLE bool saveAs(const QString &path);
    Q_INVOKABLE void clear();

signals:
    void pathChanged();
    void valueChanged();
    void dirtyChanged();
    void watchChanged();
    void autoReloadChanged();
    void compactChanged();
    void errorStringChanged();
    void loaded();
    void saved();
    void externalChanged(const QString &path);

private:
    bool parseData(const QByteArray &data);
    void setDirty(bool dirty);
    void setErrorString(const QString &error);

    FileDocument *m_file = nullptr;
    QVariant m_value;
    bool m_dirty = false;
    bool m_compact = false;
    QString m_errorString;
};

} // namespace LfQuick
