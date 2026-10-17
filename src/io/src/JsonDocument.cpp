#include <LfQuick/Io/JsonDocument.h>

#include <LfQuick/Io/FileDocument.h>

#include <QJsonDocument>
#include <QJsonParseError>

namespace LfQuick {

JsonDocument::JsonDocument(QObject *parent)
    : QObject(parent)
    , m_file(new FileDocument(this))
{
    connect(m_file, &FileDocument::pathChanged, this, &JsonDocument::pathChanged);
    connect(m_file, &FileDocument::watchChanged, this, &JsonDocument::watchChanged);
    connect(m_file, &FileDocument::autoReloadChanged,
            this, &JsonDocument::autoReloadChanged);
    connect(m_file, &FileDocument::reloadWhenDirtyChanged,
            this, &JsonDocument::reloadWhenDirtyChanged);
    connect(m_file, &FileDocument::externalConflictChanged,
            this, &JsonDocument::externalConflictChanged);
    connect(m_file, &FileDocument::externalChanged,
            this, &JsonDocument::externalChanged);
    connect(m_file, &FileDocument::externalConflictDetected,
            this, &JsonDocument::externalConflictDetected);
    connect(m_file, &FileDocument::loaded, this, [this] {
        if (parseData(m_file->data()))
            emit loaded();
    });
}

QString JsonDocument::path() const
{
    return m_file ? m_file->path() : QString{};
}

void JsonDocument::setPath(const QString &path)
{
    if (m_file)
        m_file->setPath(path);
}

void JsonDocument::setValue(const QVariant &value)
{
    if (m_value == value)
        return;
    m_value = value;
    setDirty(true);
    if (m_file)
        m_file->markDirty();
    setErrorString({});
    emit valueChanged();
}

bool JsonDocument::watch() const
{
    return m_file && m_file->watch();
}

void JsonDocument::setWatch(bool watch)
{
    if (m_file)
        m_file->setWatch(watch);
}

bool JsonDocument::autoReload() const
{
    return m_file && m_file->autoReload();
}

void JsonDocument::setAutoReload(bool autoReload)
{
    if (m_file)
        m_file->setAutoReload(autoReload);
}

bool JsonDocument::reloadWhenDirty() const
{
    return m_file && m_file->reloadWhenDirty();
}

void JsonDocument::setReloadWhenDirty(bool reloadWhenDirty)
{
    if (m_file)
        m_file->setReloadWhenDirty(reloadWhenDirty);
}

bool JsonDocument::externalConflict() const
{
    return m_file && m_file->externalConflict();
}

void JsonDocument::setCompact(bool compact)
{
    if (m_compact == compact)
        return;
    m_compact = compact;
    emit compactChanged();
}

bool JsonDocument::load()
{
    if (!m_file) {
        setErrorString(QStringLiteral("File document is unavailable"));
        return false;
    }

    if (!m_file->load()) {
        setErrorString(m_file->errorString());
        return false;
    }

    // FileDocument::loaded is emitted synchronously after a successful read;
    // the connected parser owns the JSON validation and error state.
    return m_errorString.isEmpty();
}

bool JsonDocument::save()
{
    if (!m_file) {
        setErrorString(QStringLiteral("File document is unavailable"));
        return false;
    }

    const QJsonDocument document = QJsonDocument::fromVariant(m_value);
    if (document.isNull()) {
        setErrorString(QStringLiteral("JSON root must be an object or array"));
        return false;
    }

    m_file->setData(document.toJson(m_compact ? QJsonDocument::Compact
                                              : QJsonDocument::Indented));
    if (!m_file->save()) {
        setErrorString(m_file->errorString());
        return false;
    }

    setDirty(false);
    setErrorString({});
    emit saved();
    return true;
}

bool JsonDocument::saveAs(const QString &path)
{
    if (!m_file)
        return false;
    setPath(path);
    return save();
}

void JsonDocument::clear()
{
    if (!m_value.isValid() && !m_dirty)
        return;
    m_value = {};
    setDirty(true);
    if (m_file)
        m_file->markDirty();
    setErrorString({});
    emit valueChanged();
}

bool JsonDocument::parseData(const QByteArray &data)
{
    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(data, &parseError);
    if (parseError.error != QJsonParseError::NoError || document.isNull()) {
        setErrorString(parseError.error == QJsonParseError::NoError
                           ? QStringLiteral("JSON root must be an object or array")
                           : QStringLiteral("%1 at offset %2")
                                 .arg(parseError.errorString())
                                 .arg(parseError.offset));
        return false;
    }

    const QVariant parsed = document.toVariant();
    const bool changed = m_value != parsed;
    m_value = parsed;
    setDirty(false);
    setErrorString({});
    if (changed)
        emit valueChanged();
    return true;
}

void JsonDocument::setDirty(bool dirty)
{
    if (m_dirty == dirty)
        return;
    m_dirty = dirty;
    emit dirtyChanged();
}

void JsonDocument::setErrorString(const QString &error)
{
    if (m_errorString == error)
        return;
    m_errorString = error;
    emit errorStringChanged();
}

} // namespace LfQuick
