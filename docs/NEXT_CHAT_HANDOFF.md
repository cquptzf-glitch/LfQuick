# LfQuick 新窗口开发交接

> 交接日期：2026-10-16  
> 仓库：`https://github.com/cquptzf-glitch/LfQuick`  
> Git 策略：**main-only**，不要创建普通 feature branch。

## 1. 开场直接做什么

不要重新做需求分析。直接：

1. 读取 `main` 最新 HEAD；
2. 读取本文件、`docs/IMPLEMENTATION_STATUS.md`、详细开发计划、V2.1 架构审查和 REFERENCE_POLICY；
3. 检查最新 GitHub Actions；
4. 如果出现真实 Configure / Build / Test steps，先修 Qt 6.8.3 + MSVC2022；
5. 如果仍然 `steps = null`，明确为 runner/infrastructure issue，继续下面开发顺序。

## 2. 当前功能基线

最新功能 HEAD：

```text
7c54f74d8ffd5ec3926b822254b09930bd8c1884
feat(demo): exercise DataGrid state and edit validation
```

本批提交：

```text
1b835b0 feat(dataview): persist stable DataGrid column state
19129fd feat(dataview): add validated edit transactions
7c54f74 feat(demo): exercise DataGrid state and edit validation
```

上一批：

```text
013053b feat(runtime): add cross-provider page fallback
ab06e2c fix(runtime): keep provider fallback ABI incomplete-type safe
703cbc1 feat(ui): add QML-friendly navigation guard controller
7bc28f7 feat(widgetbridge): add QWidget QML focus bridge
```

## 3. 当前关键能力

### Runtime / Migration

- PageRouter / Lifecycle / History
- None / KeepAlive / Lru cache
- INavigationGuard
- Router-level cross-provider fallback
- provider diagnostics
- QML failure -> WidgetPageProvider -> legacy QWidget
- final provider-native ErrorPage

Runtime 不感知 QWidget/QML 技术栈。

### Quick/UI

- QuickRuntime / QuickPageProvider
- QmlErrorCollector / PageLoadError
- Overlay / Busy
- FocusNavigator
- NavigationGuardState / NavigationGuardController
- confirm/cancel retry-once flow

### WidgetBridge

- QuickSurface
- WidgetPageProvider
- WidgetFocusBridge / FocusTransferPolicy
- QWidget ↔ QML Tab/Backtab
- focus remember/restore
- scanner isolation
- modal focus trap

### DataView — 本批新增

#### ColumnStateModel

通用列状态：

```text
stable id
modelColumn
title
width
visible
visual order
```

恢复规则以 stable id 为准。旧 model column index 不作为身份；模型列顺发生变化时，配置仍按 id 迁移。

#### ColumnConfigStore

配置 key：

```text
tableId
schemaVersion
modelSchema
```

当前 backend：QSettings，测试可指定独立 INI filePath。payload 为 JSON column list。

后续可抽 `IColumnConfigBackend`，但当前 API 不依赖任何医疗/product 字段。

#### DataGrid

兼容旧 `columnWidths[]`，同时新增 ColumnStateModel 路径。

使用 Qt 6.8 public API：

```text
moveColumn
columnMoved
clearColumnReordering
columnWidthProvider
```

支持：

- width persistence
- order persistence
- visibility persistence
- schema-safe restore

#### DataEditTransaction

流程：

```text
begin(row, logicalColumn)
  ↓
temporaryValue
  ↓
acceptValidation(...)
  ├─ invalid -> commit rejected / rollback
  └─ valid -> commit -> model.setData(Qt::EditRole)
```

重要：DataGrid 列移动后，transaction / validator / signals 使用 **logical model column**，不能使用 visual column。

DataGrid properties/signals：

```text
editTransaction
editValidator
editCommitted
editRejected
```

### Medical Demo Results

已验证：

- 7 stable result column ids
- width/order/visibility persistence
- hide/show Flag
- Reset Columns
- DataEditTransaction
- numeric result validation
- commit/reject UI feedback

## 4. Tests

`tst_dataview` 新增：

- stable-id restore across model-column changes
- new column keeps schema default
- persistence round-trip
- schemaVersion/modelSchema mismatch
- commit blocked before validation
- validation failure keeps source model untouched
- rollback keeps original value

这些仍然 **没有真实 Qt 6.8.3 + MSVC2022 CI 编译结果**。

## 5. CI 状态

最新功能 run：

```text
run: 38893152500
head: 7c54f74d8ffd5ec3926b822254b09930bd8c1884
job: windows-qt683
status: completed
conclusion: failure
steps: null
logs_url: null
```

仍为基础设施异常。不要宣称 CI Green，也不要宣称源码编译失败。

Runner 恢复后优先检查：

- ColumnStateModel QML_ELEMENT / moc
- QSettings/QJsonDocument persistence
- DataEditTransaction QAbstractItemModel* property
- Qt 6.8 TableView moveColumn/columnMoved
- edit delegate / model.edit writable behavior
- QSortFilterProxyModel setData forwarding
- ResultsPage QML registration
- 上一批 fallback/guard/focus 相关 MSVC/QML 问题

## 6. 下一开发批次

### Step 0 — Build gate

每次开工先查 Actions。若有实际 steps，先修构建测试。

### Step 1 — DataGrid typed delegates

继续优先于 NavigationView：

```text
CheckDelegate
ComboDelegate
ProgressDelegate
IconDelegate
EditorDelegate contract
```

要求：

- delegate contract 不依赖 Sample/Patient/Reagent；
- typed editor 要继续走 DataEditTransaction；
- validation 由调用层提供；
- 不让 delegate 自己存长期业务状态。

### Step 2 — Selection / actions / keyboard

实现：

- batch row selection helpers
- selected row/index export
- context-menu request contract
- row/cell action contract
- cross-table keyboard navigation
- Home/End/PageUp/PageDown 等可验证行为

### Step 3 — Performance

建立：

```text
20k rows
50k rows
8 / 20 / 40 columns
update 1 / 10 / 100 per sec
```

先记录：

- model population time
- filter/sort time
- first population
- memory baseline（可获取时）
- QML scroll/frame 指标后续在 GUI benchmark 中补

不要设置脆弱的绝对性能断言，先输出 benchmark telemetry。

### Step 4 — Medical Demo

继续：

```text
Sample Register
Sample Rack
Results Review
```

Results Review 最终验证：

```text
DataGrid
+ NavigationGuard
+ Permission
+ Validation
+ Overlay
```

### Step 5 — 后续

- Permission attached API
- Validation RuleTable
- Toast/Dialog dual-stack
- Focus Bridge GUI stress
- WindowRouter
- NavigationView（不优先巨大版本）
- Charts
- Diagnostics
- Hot Reload
- Qt 6.12 adapter

## 7. 已知债务

- Singleton 非 route-global singleton。
- LRU 无 memory weight。
- NavigationGuardController 单 pending request。
- Focus Bridge boundary MVP。
- ColumnConfigStore backend 目前内置 QSettings/INI，后续可抽 interface。
- typed DataGrid delegates 尚未完成。
- batch/context/cross-table nav 尚未完成。
- 20k/50k benchmark 尚未建立。
- CI 尚未实编。

## 8. 架构边界

```text
LfQuickCore      -> QtCore only
LfQuickRuntime   -> Core，不允许 QWidget/QQuickItem
LfQuickUi        -> Core + Runtime + Qt Quick
LfQuickDataView  -> Ui + Qt Model/View/Quick
LfQuickPlatform  -> platform/input abstraction
WidgetBridge     -> optional Qt Widgets / QuickWidgets
Integration.*    -> legacy/product-specific adapters
```

不要把 EventFunc / LfWidget.dll / WindowManager / QXlsx / LimeReport / HookProc / ScanerHook / Patient/Sample/Reagent domain / medical Permission IDs / QC-Calibration 算法写进 Core/DataGrid。

继续遵守 `docs/REFERENCE_POLICY.md`，FluentUI2/Pro 只做 public API/behavior clean-room 参考。
