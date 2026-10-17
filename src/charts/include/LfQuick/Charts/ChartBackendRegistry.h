#pragma once

#include <LfQuick/Charts/ChartDescriptor.h>
#include <LfQuick/Charts/LfQuickChartsGlobal.h>

#include <QObject>
#include <QStringList>
#include <QVariantList>
#include <QVariantMap>

namespace LfQuick {

struct LFQUICKCHARTS_EXPORT ChartBackendDescriptor
{
    QString id;
    QString name;
    QString technology;
    int priority = 0;
    bool available = true;
    QList<ChartSeriesType> seriesTypes;
    QVariantMap metadata;

    bool isValid() const noexcept { return !id.trimmed().isEmpty(); }
    bool supports(ChartSeriesType type) const;
    QVariantMap toVariantMap() const;
    static ChartBackendDescriptor fromVariantMap(const QVariantMap &map);
};

class LFQUICKCHARTS_EXPORT ChartBackendRegistry : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int count READ count NOTIFY backendsChanged FINAL)
    Q_PROPERTY(QVariantList backends READ backends NOTIFY backendsChanged FINAL)

public:
    explicit ChartBackendRegistry(QObject *parent = nullptr);

    int count() const noexcept { return m_backends.size(); }
    QVariantList backends() const;

    bool registerBackend(ChartBackendDescriptor descriptor);
    bool unregisterBackend(const QString &id);

    Q_INVOKABLE bool registerBackendMap(const QVariantMap &descriptor);
    Q_INVOKABLE bool unregisterBackendMap(const QString &id);
    Q_INVOKABLE bool contains(const QString &id) const;
    Q_INVOKABLE QVariantMap backend(const QString &id) const;
    Q_INVOKABLE QVariantMap resolve(int seriesType,
                                    const QString &preferredBackend = {}) const;
    Q_INVOKABLE QStringList availableBackendIds(int seriesType = -1) const;
    Q_INVOKABLE void clear();

signals:
    void backendsChanged();

private:
    static QString normalizedId(const QString &id);

    QList<ChartBackendDescriptor> m_backends;
};

} // namespace LfQuick
