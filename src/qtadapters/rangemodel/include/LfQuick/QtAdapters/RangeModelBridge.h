#pragma once

#include <LfQuick/QtAdapters/LfQuickRangeModelAdapterGlobal.h>

#include <QAbstractItemModel>
#include <QObject>
#include <QStringList>
#include <QVariantList>
#include <QVariantMap>

#include <memory>

namespace LfQuick {

class LFQUICKRANGEMODELADAPTER_EXPORT RangeModelBridge : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QAbstractItemModel* model READ model NOTIFY modelChanged FINAL)
    Q_PROPERTY(QStringList roleKeys READ roleKeys WRITE setRoleKeys NOTIFY roleKeysChanged FINAL)
    Q_PROPERTY(QVariantList rows READ rows WRITE setRows NOTIFY rowsChanged FINAL)
    Q_PROPERTY(bool dragDropSupported READ dragDropSupported CONSTANT FINAL)
    Q_PROPERTY(bool dragDropEnabled READ dragDropEnabled NOTIFY dragDropEnabledChanged FINAL)

public:
    explicit RangeModelBridge(QObject *parent = nullptr);
    ~RangeModelBridge() override;

    QAbstractItemModel *model() const noexcept;

    QStringList roleKeys() const;
    void setRoleKeys(const QStringList &keys);

    QVariantList rows() const;
    void setRows(const QVariantList &rows);

    bool dragDropSupported() const noexcept;
    bool dragDropEnabled() const noexcept;

    Q_INVOKABLE bool setDragDropEnabled(bool enabled);
    Q_INVOKABLE QVariantMap rowAt(int row) const;
    Q_INVOKABLE void clear();

signals:
    void modelChanged();
    void roleKeysChanged();
    void rowsChanged();
    void dragDropEnabledChanged();

private:
    class Private;
    std::unique_ptr<Private> d;

    void rebuildModel();
};

} // namespace LfQuick
