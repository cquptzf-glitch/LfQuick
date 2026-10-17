#pragma once

#include <LfQuick/Models/LfQuickModelsGlobal.h>

#include <QAbstractListModel>
#include <QJSValue>
#include <QString>
#include <QtQmlIntegration/qqmlintegration.h>

#include <optional>

namespace LfQuick {

class LFQUICKMODELS_EXPORT ScriptModel : public QAbstractListModel
{
    Q_OBJECT
    QML_NAMED_ELEMENT(LfScriptModel)
    Q_PROPERTY(QList<QJSValue> values READ values WRITE setValues NOTIFY valuesChanged FINAL)
    Q_PROPERTY(QString keyProperty READ keyProperty WRITE setKeyProperty NOTIFY keyPropertyChanged FINAL)
    Q_PROPERTY(ComparisonMode comparisonMode READ comparisonMode WRITE setComparisonMode NOTIFY comparisonModeChanged FINAL)
    Q_PROPERTY(int count READ rowCount NOTIFY countChanged FINAL)

public:
    enum class ComparisonMode : quint8 {
        Identity,
        Structure
    };
    Q_ENUM(ComparisonMode)

    explicit ScriptModel(QObject *parent = nullptr);

    QList<QJSValue> values() const;
    void setValues(const QList<QJSValue> &values);

    QString keyProperty() const { return m_keyProperty; }
    void setKeyProperty(const QString &property);

    ComparisonMode comparisonMode() const noexcept { return m_comparisonMode; }
    void setComparisonMode(ComparisonMode mode);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE QVariant get(int index) const;

signals:
    void valuesChanged();
    void keyPropertyChanged();
    void comparisonModeChanged();
    void countChanged();

private:
    bool valuesEqual(const QJSValue &left, const QJSValue &right) const;
    bool containsDuplicateKeys(const QList<QJSValue> &values) const;
    bool updateValuesUnique(const QList<QJSValue> &values);
    void resetValues(const QList<QJSValue> &values);

    QList<QJSValue> m_values;
    QString m_keyProperty;
    ComparisonMode m_comparisonMode = ComparisonMode::Structure;
    bool m_modifying = false;
    std::optional<QList<QJSValue>> m_stagedValues;
};

} // namespace LfQuick
