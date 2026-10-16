# LfQuick 当前实现状态

> 更新时间：2026-10-16  
> 开发策略：**main-only**。不为普通功能创建新分支；已有旧分支只做审查，不把被 main supersede 的早期实现重新合入。

## 当前基线

- Qt：6.8.3
- C++：20
- CMake：3.24+
- 目标升级：Qt 6.12 LTS
- 当前主线：`main`
- 本批功能 HEAD：`7c54f74d8ffd5ec3926b822254b09930bd8c1884`

## 本批新增提交

```text
1b835b0 feat(dataview): persist stable DataGrid column state
19129fd feat(dataview): add validated edit transactions
7c54f74 feat(demo): exercise DataGrid state and edit validation
```

上一批关键提交：

```text
013053b feat(runtime): add cross-provider page fallback
ab06e2c fix(runtime): keep provider fallback ABI incomplete-type safe
703cbc1 feat(ui): add QML-friendly navigation guard controller
7bc28f7 feat(widgetbridge): add QWidget QML focus bridge
```

## Repository / Git

- main-only workflow 持续执行。
- 旧分支 `feat/runtime-foundation` 已确认属于被 main 覆盖的早期实现，不重新合入。
- 每批功能保持语义清晰小提交。

## Core

已完成：Error / Result / OperationHandle / Permission / Validation / Command / Event / Route contracts。

边界保持：`LfQuickCore -> QtCore only`。

## Runtime

已完成：

- PageDescriptor / PageRegistry / PageProviderRegistry
- IPageProvider / IPageInstance
- PageLifecycle
- PageRouter push / replace / back / reset
- INavigationGuard
- None / KeepAlive / Lru cache
- Router 级 cross-provider fallback
- provider diagnostics / ErrorPage final fallback

Runtime 继续保持 provider-id only，不写死 QWidget/QML/LfWidget/UnionLab。

## UI / Quick Runtime

已完成：

- QuickRuntime / QuickPageProvider
- QmlErrorCollector / PageLoadError
- Overlay / Busy
- FocusNavigator
- NavigationGuardState / NavigationGuardController
- confirm/cancel/hard-block navigation workflow

Guard 不直接弹 Dialog；UI 负责确认交互。

## WidgetBridge

已完成：

- QuickSurface
- WidgetPageProvider
- WidgetFocusBridge / FocusTransferPolicy
- QWidget ↔ QML Tab/Backtab boundary transfer
- remember/restore focus
- scannerIsolation / modalFocusTrap

## Hybrid Demo

当前验证：

1. QML ↔ Legacy QWidget 共用 Router/History；
2. Broken QML → Router → WidgetPageProvider → legacy fallback；
3. legacy fallback 失败 → provider-native ErrorPage；
4. dirty guard confirm/cancel；
5. busy guard hard block；
6. QLineEdit → QML TextField → QLineEdit Tab/Backtab；
7. scanner focus isolation；
8. route back focus restore 基础。

## DataView

### 基础能力

已有：

- DataTableModel
- DataFilterProxyModel
- DataGrid / TableView MVP
- synchronized header
- row selection
- keyboard navigation
- movable / resizable columns
- 10k / 5k Medical Demo synthetic data

### DataGrid 第二阶段：本批完成

新增 `ColumnStateModel`：

```text
stable id
modelColumn
width
visible
visual order
```

关键语义：历史配置按 **stable column id** 合并，而不是按旧 model column index 恢复。因此模型列序变化后，只要 stable id 不变，width/visibility/order 仍可正确恢复；新列保留当前 schema 默认配置。

新增 `ColumnConfigStore`：

- tableId
- schemaVersion
- modelSchema
- QSettings 默认存储
- 可指定 INI filePath，便于测试或产品隔离
- JSON column-state payload
- schema/modelSchema mismatch 自动拒绝旧配置

DataGrid 已接入 Qt 6.8 正式列重排 API：

- `moveColumn()`
- `columnMoved(logicalIndex, oldVisualIndex, newVisualIndex)`
- `clearColumnReordering()`
- hidden column via columnWidthProvider = 0
- width state capture + restore

旧 `columnWidths[]` 路径继续保留，未使用 ColumnStateModel 时兼容原调用方式。

### Edit Transaction

新增 `DataEditTransaction`：

```text
Begin
  ↓
Temporary value
  ↓
Validate
  ├─ Invalid -> Rollback / model unchanged
  └─ Valid   -> Commit -> QAbstractItemModel::setData
```

状态：

- active
- row / column
- originalValue
- temporaryValue
- ValidationState::NotValidated / Valid / Invalid
- validationMessage

DataGrid 新增：

- `editTransaction`
- `editValidator`
- `editCommitted`
- `editRejected`

编辑时先写 temporary value，validator 通过后才修改 model。列重排后事务使用 logical/model column，而不是 TableView visual column，避免移动列后写错业务字段。

### Tests

`tst_dataview` 新增：

- stable id 恢复时不依赖历史 index
- new schema column 保留默认状态
- ColumnConfigStore round-trip
- schemaVersion mismatch rejection
- modelSchema mismatch rejection
- 未 Validate 不允许 Commit
- validation failure 不修改 model
- Rollback 保持原数据

### 下一步仍未完成

- CheckDelegate
- ComboDelegate
- ProgressDelegate
- IconDelegate
- 可插拔 EditorDelegate contract
- batch selection
- context menu
- keyboard / cross-table navigation
- 20k / 50k row stress benchmark
- persistence backend interface（当前 ColumnConfigStore 内置 QSettings/INI；后续可抽 IColumnConfigBackend 供产品数据库/配置中心替换）

## Medical Demo

Results 页面已开始从“展示 DataGrid”升级为框架实战页：

- Result 7 列建立 stable ids
- width/order/visibility persistence
- Flag hide/show
- Reset Columns
- DataEditTransaction
- Result 数值 validator（0~9999）
- edit commit/reject status feedback

当前数据仍为 synthetic demo 数据，不把 Sample/Result 领域模型写入 DataView。

下一批真实页面仍为：

- Sample Register
- Sample Rack
- Results Review

Results Review 后续继续组合 NavigationGuard / Permission / Validation / Overlay。

## Platform

已有 ScannerSession / KeyboardWedgeScanner / InputRouter。

## CI 当前状态

本批最新 GitHub Actions：

```text
run: 38893152500
head: 7c54f74d8ffd5ec3926b822254b09930bd8c1884
job: windows-qt683
status: completed
conclusion: failure
steps: null
logs_url: null
```

仍属于 GitHub Actions runner / infrastructure abnormal：没有 checkout / Install Qt / Configure / Build / Test。

因此当前结论必须保持：

```text
Architecture/source implementation: progressed
Workflow triggered: yes
Real Qt 6.8.3 + MSVC2022 compile: NOT EXECUTED
CI Green: NO CLAIM
Source compile failure: NO COMPILER EVIDENCE
```

Runner 一旦恢复并出现真实 steps，第一优先级修 Configure / Build / CTest。

本批重点实编检查：

- ColumnStateModel QML registrar / QVariantList API
- ColumnConfigStore QSettings + QJsonDocument conversion
- DataEditTransaction QAbstractItemModel* Q_PROPERTY / moc
- TableView 6.8 moveColumn/columnMoved QML syntax
- edit delegate model.edit write path
- QSortFilterProxyModel edit forwarding
- ResultsPage QML ColumnStateModel lifecycle / persistence timer

## 已知设计债务

- PageCachePolicy::Singleton 尚无 route-global singleton 去重。
- LRU 尚无 memory cost/weight。
- NavigationGuardController 为单 pending request。
- WidgetFocusBridge 仍是 boundary MVP，需要真实 GUI stress。
- ColumnConfigStore 后续建议抽 storage backend interface。
- DataGrid 当前 default editor 是 TextField；typed delegates 尚未实现。
- DataGrid selection/action contract 还不完整。
- CI 尚无真实 Qt/MSVC 编译结果。

## 下一开发顺序

1. **Build gate**：只要 runner 恢复，立即修真实编译/测试问题。
2. DataGrid typed delegates：Check / Combo / Progress / Icon / Editor contract。
3. DataGrid batch selection + context menu + keyboard/cross-table navigation。
4. 20k / 50k row benchmark + update-rate stress。
5. Medical Demo：Sample Register / Sample Rack / Results Review。
6. Permission attached API + Validation RuleTable + Overlay/Dialog 双栈一致性。
7. 再考虑 WindowRouter / NavigationView / Charts / Diagnostics / Hot Reload。

## 架构边界继续保持

```text
LfQuickCore      -> QtCore only
LfQuickRuntime   -> Core，不允许 QWidget/QQuickItem
LfQuickUi        -> Core + Runtime + Qt Quick
LfQuickDataView  -> Ui + Qt Model/View/Quick
LfQuickPlatform  -> platform/input abstraction
WidgetBridge     -> optional Qt Widgets / QuickWidgets
Integration.*    -> legacy/product-specific adapters
```

Core 禁止 EventFunc / LfWidget.dll / WindowManager / QXlsx / LimeReport / HookProc / ScanerHook / Patient-Sample-Reagent domain / medical permission IDs / QC-Calibration algorithms。

外部参考继续遵守 `docs/REFERENCE_POLICY.md`。
