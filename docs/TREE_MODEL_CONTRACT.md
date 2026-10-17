# LfQuick Tree Model contract (Qt 6.8.3)

## Scope / module boundary
- Import `LfQuick.DataView` for `LfTreeView` and `LfTreeSelect`; `LfQuick` supplies theme and basic controls.
- Both tree controls require a **hierarchical C++ QAbstractItemModel**. A JavaScript array or a flat QML ListModel is **not** a tree model.
- The default delegate displays `Qt::DisplayRole` in column 0. Application-provided models own node lifetime and values.
- Public `nodeActivated(QModelIndex, label)` and `itemSelected(QModelIndex, label)` expose **source-model QModelIndex**, not flattened visual-row integers.
- Expansion APIs use **visual row indices**: `expand(row)`, `collapse(row)`, `activateRow(viewRow, label)`. Use `rowAtIndex(sourceIndex)` when you need to convert a source model index.
- A tree can be lazy: implement `canFetchMore(parent)` / `fetchMore(parent)` in your model. `expandRecursively` does **not** trigger fetchMore.
- Use stable app-level node IDs in your model when selection must survive a model reset; QModelIndex is invalid after reset. Do not serialize QModelIndex as application state.
- Selection is owned by the view's `ItemSelectionModel`. TreeSelect tracks the chosen index plus a snapshot of its display label; on model reset, the caller should clear or re-resolve its choice.

## C++ sample

```cpp
QStandardItemModel tree;
tree.setHorizontalHeaderLabels({"Name"});
auto *instrument = new QStandardItem("Instrument A");
instrument->appendRow(new QStandardItem("Channel 1"));
instrument->appendRow(new QStandardItem("Channel 2"));
tree.appendRow(instrument);
engine.rootContext()->setContextProperty("instrumentTree", &tree);
```

## QML sample

```qml
import QtQuick
import LfQuick.DataView

LfTreeView {
    width: 320
    height: 360
    model: instrumentTree
    onNodeActivated: function(modelIndex, label) {
        console.log("Selected", label, "source row:", modelIndex.row)
    }
}
```

## Other selectors
- `LfSearchableComboBox` and `LfMultiSelect` belong to `LfQuick` and work with an ordinary JS array or QML ListModel. They are not QAbstractItemModel tree adapters.
- For `LfMultiSelect`, provide object `value` keys (or configure `valueRole`) that remain unique and stable. Use `selectedValues` and `selectionCommitted(values)` for external state.
- `LfSearchableComboBox` filters via `textAt(index)` and preserves source indices in `activated(index)`.
- Async tree searching, cross-parent filtering, persistent multi-tree selection, and arbitrary role-name adapters are not implemented in this batch.

- `LfTransferList` transfers stable array/ListModel keys, with `addIndex`, `removeKey`, `moveSelectedToTarget`, `moveSelectedToSource`, and bulk helpers. Disabled source options cannot be added. Use bounded datasets (the display lists calculate indices in JavaScript).
