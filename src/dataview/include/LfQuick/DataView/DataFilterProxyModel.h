#pragma once

#include <LfQuick/DataView/LfQuickDataViewGlobal.h>

#include <QSortFilterProxyModel>
#include <QtQml/qqmlintegration.h>

namespace LfQuick {

class LFQUICKDATAVIEW_EXPORT DataFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(QString filterText READ filterText WRITE setFilterText NOTIFY filterTextChanged FINAL)
    Q_PROPERTY(QString filterKey READ filterKey WRITE setFilterKey NOTIFY filterKeyChanged FINAL)
    Q_PROPERTY(bool caseSensitive READ caseSensitive WRITE setCaseSensitive NOTIFY caseSensitiveChanged FINAL)

public:
    explicit DataFilterProxyModel(QObject *parent = nullptr);

    QString filterText() const;
    void setFilterText(const QString &text);

    QString filterKey() const;
    void setFilterKey(const QString &key);

    bool caseSensitive() const;
    void setCaseSensitive(bool value);

    Q_INVOKABLE int mapRowToSource(int proxyRow) const;
    Q_INVOKABLE int mapRowFromSource(int sourceRow) const;

signals:
    void filterTextChanged();
    void filterKeyChanged();
    void caseSensitiveChanged();

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;

private:
    QString m_filterText;
    QString m_filterKey;
    bool m_caseSensitive = false;
};

} // namespace LfQuick
