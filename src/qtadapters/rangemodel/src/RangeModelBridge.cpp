#include <LfQuick/QtAdapters/RangeModelBridge.h>

#include <QAbstractItemModel>
#include <QHash>
#include <QRangeModel>
#include <QSet>

#include <utility>

namespace LfQuick {

class RangeModelBridge::Private
{
public:
    QList<QVariantMap> rows;
    QStringList roleKeys;
    std::unique_ptr<QRangeModel> model;
    bool dragDropEnabled = false;
};

RangeModelBridge::RangeModelBridge(QObject *parent)
    : QObject(parent)
    , d(std::make_unique<Private>())
{
    rebuildModel();
}

RangeModelBridge::~RangeModelBridge() = default;

QAbstractItemModel *RangeModelBridge::model() const noexcept
{
    return d->model.get();
}

QStringList RangeModelBridge::roleKeys() const
{
    return d->roleKeys;
}

void RangeModelBridge::setRoleKeys(const QStringList &keys)
{
    QStringList normalized;
    QSet<QString> seen;
    normalized.reserve(keys.size());
    for (const QString &key : keys) {
        const QString value = key.trimmed();
        if (value.isEmpty() || seen.contains(value))
            continue;
        seen.insert(value);
        normalized.push_back(value);
    }

    if (d->roleKeys == normalized)
        return;

    d->roleKeys = normalized;
    rebuildModel();
    emit roleKeysChanged();
}

QVariantList RangeModelBridge::rows() const
{
    QVariantList result;
    result.reserve(d->rows.size());
    for (const QVariantMap &row : d->rows)
        result.push_back(row);
    return result;
}

void RangeModelBridge::setRows(const QVariantList &rowsValue)
{
    QList<QVariantMap> converted;
    converted.reserve(rowsValue.size());

    QStringList discoveredKeys;
    QSet<QString> seenKeys;
    for (const QVariant &value : rowsValue) {
        const QVariantMap row = value.toMap();
        converted.push_back(row);
        for (auto it = row.cbegin(); it != row.cend(); ++it) {
            if (!seenKeys.contains(it.key())) {
                seenKeys.insert(it.key());
                discoveredKeys.push_back(it.key());
            }
        }
    }

    const bool adoptKeys = d->roleKeys.isEmpty() && !discoveredKeys.isEmpty();
    d->rows = std::move(converted);
    if (adoptKeys)
        d->roleKeys = discoveredKeys;

    rebuildModel();
    if (adoptKeys)
        emit roleKeysChanged();
    emit rowsChanged();
}

bool RangeModelBridge::dragDropSupported() const noexcept
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 12, 0)
    return true;
#else
    return false;
#endif
}

bool RangeModelBridge::dragDropEnabled() const noexcept
{
    return d->dragDropEnabled;
}

bool RangeModelBridge::setDragDropEnabled(bool enabled)
{
#if QT_VERSION < QT_VERSION_CHECK(6, 12, 0)
    if (enabled)
        return false;
#endif

    if (d->dragDropEnabled == enabled)
        return true;

    d->dragDropEnabled = enabled;
#if QT_VERSION >= QT_VERSION_CHECK(6, 12, 0)
    if (d->model) {
        const Qt::DropActions actions = enabled
            ? Qt::DropActions(Qt::CopyAction | Qt::MoveAction)
            : Qt::DropActions{};
        d->model->setSupportedDragActions(actions);
        d->model->setSupportedDropActions(actions);
    }
#endif
    emit dragDropEnabledChanged();
    return true;
}

QVariantMap RangeModelBridge::rowAt(int row) const
{
    if (row < 0 || row >= d->rows.size())
        return {};
    return d->rows.at(row);
}

void RangeModelBridge::clear()
{
    if (d->rows.isEmpty())
        return;
    d->rows.clear();
    rebuildModel();
    emit rowsChanged();
}

void RangeModelBridge::rebuildModel()
{
    auto model = std::make_unique<QRangeModel>(&d->rows);

    QHash<int, QByteArray> roleNames;
    int role = Qt::UserRole + 1;
    for (const QString &key : std::as_const(d->roleKeys))
        roleNames.insert(role++, key.toUtf8());
    model->setRoleNames(roleNames);

#if QT_VERSION >= QT_VERSION_CHECK(6, 12, 0)
    if (d->dragDropEnabled) {
        const Qt::DropActions actions = Qt::CopyAction | Qt::MoveAction;
        model->setSupportedDragActions(actions);
        model->setSupportedDropActions(actions);
    }
#endif

    connect(model.get(), &QAbstractItemModel::dataChanged,
            this, [this] { emit rowsChanged(); });
    connect(model.get(), &QAbstractItemModel::rowsInserted,
            this, [this] { emit rowsChanged(); });
    connect(model.get(), &QAbstractItemModel::rowsRemoved,
            this, [this] { emit rowsChanged(); });
    connect(model.get(), &QAbstractItemModel::rowsMoved,
            this, [this] { emit rowsChanged(); });
    connect(model.get(), &QAbstractItemModel::modelReset,
            this, [this] { emit rowsChanged(); });

    d->model = std::move(model);
    emit modelChanged();
}

} // namespace LfQuick
