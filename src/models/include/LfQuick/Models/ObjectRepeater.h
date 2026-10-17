#pragma once

#include <LfQuick/Models/LfQuickModelsGlobal.h>

#include <QMetaObject>
#include <QObject>
#include <QPointer>
#include <QVariant>
#include <QQmlComponent>
#include <QQmlListProperty>
#include <QQmlParserStatus>
#include <QtQmlIntegration/qqmlintegration.h>

namespace LfQuick {

class QAbstractItemModel;

class LFQUICKMODELS_EXPORT ObjectRepeater : public QObject, public QQmlParserStatus
{
    Q_OBJECT
    QML_NAMED_ELEMENT(LfObjectRepeater)
    Q_INTERFACES(QQmlParserStatus)
    Q_CLASSINFO("DefaultProperty", "delegate")

    Q_PROPERTY(QVariant model READ model WRITE setModel NOTIFY modelChanged FINAL)
    Q_PROPERTY(QQmlComponent* delegate READ delegate WRITE setDelegate NOTIFY delegateChanged FINAL)
    Q_PROPERTY(QString keyProperty READ keyProperty WRITE setKeyProperty NOTIFY keyPropertyChanged FINAL)
    Q_PROPERTY(QQmlListProperty<QObject> instances READ instances NOTIFY instancesChanged FINAL)
    Q_PROPERTY(int count READ count NOTIFY countChanged FINAL)
    Q_PROPERTY(QString lastError READ lastError NOTIFY lastErrorChanged FINAL)

public:
    explicit ObjectRepeater(QObject *parent = nullptr);
    ~ObjectRepeater() override;

    QVariant model() const { return m_model; }
    void setModel(const QVariant &model);

    QQmlComponent *delegate() const noexcept { return m_delegate; }
    void setDelegate(QQmlComponent *delegate);

    QString keyProperty() const { return m_keyProperty; }
    void setKeyProperty(const QString &property);

    QQmlListProperty<QObject> instances();
    int count() const noexcept { return m_records.size(); }
    QString lastError() const { return m_lastError; }

    Q_INVOKABLE QObject *instanceAt(int index) const;
    Q_INVOKABLE void refresh();
    Q_INVOKABLE void clear();

    void classBegin() override;
    void componentComplete() override;

signals:
    void modelChanged();
    void delegateChanged();
    void keyPropertyChanged();
    void instancesChanged();
    void countChanged();
    void lastErrorChanged();
    void instanceCreated(QObject *instance, int index, const QVariant &modelData);
    void instanceRemoved(QObject *instance, int index);

private:
    struct Record {
        QVariant key;
        QVariant modelData;
        QObject *identity = nullptr;
        QPointer<QObject> instance;
        QMetaObject::Connection destroyedConnection;
    };

    static qsizetype instanceCount(QQmlListProperty<QObject> *property);
    static QObject *instanceValue(QQmlListProperty<QObject> *property, qsizetype index);

    QVariantList extractValues() const;
    QVariantList extractModelRows(QAbstractItemModel *model) const;
    QVariant keyFor(const QVariant &value) const;
    bool keysEqual(const QVariant &left, const QVariant &right) const;
    int findRecordByKey(const QList<Record> &records, const QVariant &key) const;
    QObject *createInstance(const QVariant &modelData, int modelIndex, QString *error);
    void reconnectModel();
    void disconnectModel();
    void destroyRecords(QList<Record> &records);
    void setLastError(const QString &error);

    QVariant m_model;
    QPointer<QQmlComponent> m_delegate;
    QMetaObject::Connection m_delegateDestroyedConnection;
    QString m_keyProperty;
    QList<Record> m_records;
    QList<QMetaObject::Connection> m_modelConnections;
    QString m_lastError;
    bool m_complete = true;
};

} // namespace LfQuick
