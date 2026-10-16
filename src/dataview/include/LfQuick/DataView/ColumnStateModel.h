#pragma once

#include <LfQuick/DataView/LfQuickDataViewGlobal.h>

#include <QAbstractListModel>
#include <QVariantList>
#include <QtQml/qqmlintegration.h>

namespace LfQuick {

class LFQUICKDATAVIEW_EXPORT ColumnStateModel : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(int count READ rowCount NOTIFY countChanged FINAL)

public:
    enum Role {
        IdRole = Qt::UserRole + 1,
        ModelColumnRole,
        TitleRole,
        WidthRole,
        VisibleRole,
        OrderRole
    };
    Q_ENUM(Role)

    explicit ColumnStateModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = {}) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void clear();
    Q_INVOKABLE bool setColumns(const QVariantList &columns);
    Q_INVOKABLE QVariantList snapshot() const;
    Q_INVOKABLE bool restoreState(const QVariantList &columns);

    Q_INVOKABLE int indexOfId(const QString &id) const;
    Q_INVOKABLE int modelColumnAt(int visualIndex) const;
    Q_INVOKABLE QString idForModelColumn(int modelColumn) const;
    Q_INVOKABLE QString titleForModelColumn(int modelColumn) const;
    Q_INVOKABLE qreal widthForModelColumn(int modelColumn, qreal fallback = 140.0) const;
    Q_INVOKABLE bool isModelColumnVisible(int modelColumn) const;
    Q_INVOKABLE bool setWidth(const QString &id, qreal width);
    Q_INVOKABLE bool setWidthByModelColumn(int modelColumn, qreal width);
    Q_INVOKABLE bool setVisible(const QString &id, bool visible);
    Q_INVOKABLE bool moveModelColumn(int modelColumn, int visualIndex);

signals:
    void countChanged();
    void stateChanged();

private:
    struct ColumnState {
        QString id;
        int modelColumn = -1;
        QString title;
        qreal width = 140.0;
        bool visible = true;
    };

    int indexOfModelColumn(int modelColumn) const;
    static bool parseColumn(const QVariantMap &map, ColumnState &state);

    QList<ColumnState> m_columns;
};

} // namespace LfQuick
