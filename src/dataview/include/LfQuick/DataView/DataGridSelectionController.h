#pragma once

#include <LfQuick/DataView/LfQuickDataViewGlobal.h>

#include <QItemSelectionModel>
#include <QObject>
#include <QPointer>
#include <QVariantList>
#include <QtQmlIntegration/qqmlintegration.h>

namespace LfQuick {

class LFQUICKDATAVIEW_EXPORT DataGridSelectionController : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(QItemSelectionModel* selectionModel READ selectionModel WRITE setSelectionModel NOTIFY selectionModelChanged FINAL)
    Q_PROPERTY(bool selectRows READ selectRows WRITE setSelectRows NOTIFY selectRowsChanged FINAL)
    Q_PROPERTY(int selectedRowCount READ selectedRowCount NOTIFY selectionChanged FINAL)
    Q_PROPERTY(int currentRow READ currentRow NOTIFY currentChanged FINAL)
    Q_PROPERTY(int currentColumn READ currentColumn NOTIFY currentChanged FINAL)

public:
    explicit DataGridSelectionController(QObject *parent = nullptr);

    QItemSelectionModel *selectionModel() const;
    void setSelectionModel(QItemSelectionModel *selectionModel);

    bool selectRows() const noexcept { return m_selectRows; }
    void setSelectRows(bool value);

    int selectedRowCount() const;
    int currentRow() const;
    int currentColumn() const;

    Q_INVOKABLE QVariantList selectedRowsList() const;
    Q_INVOKABLE bool isRowSelected(int row) const;
    Q_INVOKABLE bool selectRow(int row, bool additive = false);
    Q_INVOKABLE bool selectRowsList(const QVariantList &rows, bool clearExisting = true);
    Q_INVOKABLE bool selectAllRows();
    Q_INVOKABLE void clear();
    Q_INVOKABLE bool setCurrentCell(int row, int column, bool select = true);
    Q_INVOKABLE bool moveCurrent(int rowDelta, int columnDelta, bool select = true);

signals:
    void selectionModelChanged();
    void selectRowsChanged();
    void selectionChanged();
    void currentChanged();

private:
    QItemSelectionModel::SelectionFlags selectionFlags(bool additive) const;
    QAbstractItemModel *model() const;
    void disconnectSelectionModel();

    QPointer<QItemSelectionModel> m_selectionModel;
    bool m_selectRows = true;
    QMetaObject::Connection m_selectionConnection;
    QMetaObject::Connection m_currentConnection;
    QMetaObject::Connection m_destroyedConnection;
};

} // namespace LfQuick
