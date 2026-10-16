#include <LfQuick/DataView/DataGridSelectionController.h>

#include <QAbstractItemModel>
#include <QItemSelection>
#include <QSet>

#include <algorithm>

namespace LfQuick {

DataGridSelectionController::DataGridSelectionController(QObject *parent)
    : QObject(parent)
{
}

QItemSelectionModel *DataGridSelectionController::selectionModel() const
{
    return m_selectionModel;
}

void DataGridSelectionController::setSelectionModel(QItemSelectionModel *selectionModel)
{
    if (m_selectionModel == selectionModel)
        return;

    disconnectSelectionModel();
    m_selectionModel = selectionModel;

    if (m_selectionModel) {
        m_selectionConnection = connect(
            m_selectionModel, &QItemSelectionModel::selectionChanged,
            this, [this] { emit selectionChanged(); });
        m_currentConnection = connect(
            m_selectionModel, &QItemSelectionModel::currentChanged,
            this, [this] { emit currentChanged(); });
        m_destroyedConnection = connect(
            m_selectionModel, &QObject::destroyed, this, [this] {
                m_selectionModel = nullptr;
                emit selectionModelChanged();
                emit selectionChanged();
                emit currentChanged();
            });
    }

    emit selectionModelChanged();
    emit selectionChanged();
    emit currentChanged();
}

void DataGridSelectionController::setSelectRows(bool value)
{
    if (m_selectRows == value)
        return;
    m_selectRows = value;
    emit selectRowsChanged();
}

int DataGridSelectionController::selectedRowCount() const
{
    return selectedRowsList().size();
}

int DataGridSelectionController::currentRow() const
{
    return m_selectionModel && m_selectionModel->currentIndex().isValid()
        ? m_selectionModel->currentIndex().row() : -1;
}

int DataGridSelectionController::currentColumn() const
{
    return m_selectionModel && m_selectionModel->currentIndex().isValid()
        ? m_selectionModel->currentIndex().column() : -1;
}

QVariantList DataGridSelectionController::selectedRowsList() const
{
    QVariantList result;
    if (!m_selectionModel)
        return result;

    QSet<int> rows;
    const QModelIndexList indexes = m_selectionModel->selectedIndexes();
    for (const QModelIndex &index : indexes) {
        if (index.isValid())
            rows.insert(index.row());
    }

    QList<int> sorted = rows.values();
    std::sort(sorted.begin(), sorted.end());
    result.reserve(sorted.size());
    for (int row : sorted)
        result.push_back(row);
    return result;
}

bool DataGridSelectionController::isRowSelected(int row) const
{
    const QVariantList rows = selectedRowsList();
    return std::any_of(rows.cbegin(), rows.cend(), [row](const QVariant &value) {
        return value.toInt() == row;
    });
}

bool DataGridSelectionController::selectRow(int row, bool additive)
{
    QAbstractItemModel *source = model();
    if (!m_selectionModel || !source || row < 0 || row >= source->rowCount()
        || source->columnCount() <= 0) {
        return false;
    }

    const QModelIndex index = source->index(row, 0);
    m_selectionModel->select(index, selectionFlags(additive));
    m_selectionModel->setCurrentIndex(index, QItemSelectionModel::NoUpdate);
    return true;
}

bool DataGridSelectionController::selectRowsList(const QVariantList &rows, bool clearExisting)
{
    QAbstractItemModel *source = model();
    if (!m_selectionModel || !source || source->columnCount() <= 0)
        return false;

    if (clearExisting)
        m_selectionModel->clearSelection();

    bool any = false;
    int firstSelected = -1;
    for (const QVariant &value : rows) {
        bool ok = false;
        const int row = value.toInt(&ok);
        if (!ok || row < 0 || row >= source->rowCount())
            continue;
        const QModelIndex index = source->index(row, 0);
        m_selectionModel->select(index, QItemSelectionModel::Select | QItemSelectionModel::Rows);
        if (firstSelected < 0)
            firstSelected = row;
        any = true;
    }

    if (firstSelected >= 0) {
        m_selectionModel->setCurrentIndex(source->index(firstSelected, 0),
                                          QItemSelectionModel::NoUpdate);
    }
    return any;
}

bool DataGridSelectionController::selectAllRows()
{
    QAbstractItemModel *source = model();
    if (!m_selectionModel || !source || source->rowCount() <= 0 || source->columnCount() <= 0)
        return false;

    const QItemSelection selection(source->index(0, 0),
                                   source->index(source->rowCount() - 1,
                                                 source->columnCount() - 1));
    m_selectionModel->select(selection, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
    m_selectionModel->setCurrentIndex(source->index(0, 0), QItemSelectionModel::NoUpdate);
    return true;
}

void DataGridSelectionController::clear()
{
    if (!m_selectionModel)
        return;
    m_selectionModel->clearSelection();
    m_selectionModel->setCurrentIndex(QModelIndex{}, QItemSelectionModel::NoUpdate);
}

bool DataGridSelectionController::setCurrentCell(int row, int column, bool select)
{
    QAbstractItemModel *source = model();
    if (!m_selectionModel || !source || row < 0 || row >= source->rowCount()
        || column < 0 || column >= source->columnCount()) {
        return false;
    }

    const QModelIndex index = source->index(row, column);
    if (select)
        m_selectionModel->select(index, selectionFlags(false));
    m_selectionModel->setCurrentIndex(index, QItemSelectionModel::NoUpdate);
    return true;
}

bool DataGridSelectionController::moveCurrent(int rowDelta, int columnDelta, bool select)
{
    QAbstractItemModel *source = model();
    if (!m_selectionModel || !source || source->rowCount() <= 0 || source->columnCount() <= 0)
        return false;

    const int baseRow = currentRow() < 0 ? 0 : currentRow();
    const int baseColumn = currentColumn() < 0 ? 0 : currentColumn();
    const int row = qBound(0, baseRow + rowDelta, source->rowCount() - 1);
    const int column = qBound(0, baseColumn + columnDelta, source->columnCount() - 1);
    return setCurrentCell(row, column, select);
}

QItemSelectionModel::SelectionFlags DataGridSelectionController::selectionFlags(bool additive) const
{
    QItemSelectionModel::SelectionFlags flags = additive
        ? QItemSelectionModel::Select
        : QItemSelectionModel::ClearAndSelect;
    if (m_selectRows)
        flags |= QItemSelectionModel::Rows;
    return flags;
}

QAbstractItemModel *DataGridSelectionController::model() const
{
    return m_selectionModel
        ? const_cast<QAbstractItemModel *>(m_selectionModel->model())
        : nullptr;
}

void DataGridSelectionController::disconnectSelectionModel()
{
    QObject::disconnect(m_selectionConnection);
    QObject::disconnect(m_currentConnection);
    QObject::disconnect(m_destroyedConnection);
    m_selectionConnection = {};
    m_currentConnection = {};
    m_destroyedConnection = {};
}

} // namespace LfQuick
