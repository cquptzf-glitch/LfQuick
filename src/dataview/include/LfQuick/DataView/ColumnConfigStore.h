#pragma once

#include <LfQuick/DataView/LfQuickDataViewGlobal.h>

#include <QObject>
#include <QString>
#include <QVariantList>
#include <QVariantMap>
#include <QtQmlIntegration/qqmlintegration.h>

namespace LfQuick {

class LFQUICKDATAVIEW_EXPORT ColumnConfigStore : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(QString storageNamespace READ storageNamespace WRITE setStorageNamespace NOTIFY storageNamespaceChanged FINAL)
    Q_PROPERTY(QString filePath READ filePath WRITE setFilePath NOTIFY filePathChanged FINAL)

public:
    explicit ColumnConfigStore(QObject *parent = nullptr);

    QString storageNamespace() const;
    void setStorageNamespace(const QString &value);
    QString filePath() const;
    void setFilePath(const QString &value);

    Q_INVOKABLE bool save(const QString &tableId,
                          int schemaVersion,
                          const QString &modelSchema,
                          const QVariantList &columns) const;
    Q_INVOKABLE QVariantMap load(const QString &tableId,
                                 int schemaVersion,
                                 const QString &modelSchema = {}) const;
    Q_INVOKABLE void clear(const QString &tableId) const;

signals:
    void storageNamespaceChanged();
    void filePathChanged();

private:
    QString groupFor(const QString &tableId) const;

    QString m_storageNamespace = QStringLiteral("LfQuick/DataGrid");
    QString m_filePath;
};

} // namespace LfQuick
