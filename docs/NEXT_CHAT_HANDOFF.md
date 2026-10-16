# LfQuick 新窗口开发交接

> 交接日期：2026-10-16  
> 仓库：`https://github.com/cquptzf-glitch/LfQuick`  
> Git 策略：**main-only**，不要创建普通 feature branch。

## 1. 开场直接做什么

不要重新做需求分析。直接：

1. 读取 `main` 最新 HEAD；
2. 读取本文件、`docs/IMPLEMENTATION_STATUS.md`、详细开发计划、V2.1 架构审查和 `REFERENCE_POLICY.md`；
3. 检查最新 GitHub Actions；
4. 如果出现真实 Configure / Build / Test steps，先修 Qt 6.8.3 + MSVC2022；
5. 如果仍然 `steps = null`，明确为 runner/infrastructure issue，继续下面开发顺序。

## 2. 当前功能基线

最新功能 HEAD：

```text
9cdbdd2a156bd9231037d01bf3ea6e089873649d
feat(demo): add complete results review workflow
```

本批提交：

```text
67508a8 feat(dataview): add typed delegates and selection navigation
8bc8926 perf(dataview): add opt-in 20k 50k benchmark matrix
dd83729 feat(ui): add QML permission evaluator
2420b4f feat(dataview): expose proxy row mapping
9cdbdd2 feat(demo): add complete results review workflow
```

上一批：

```text
1b835b0 feat(dataview): persist stable DataGrid column state
19129fd feat(dataview): add validated edit transactions
7c54f74 feat(demo): exercise DataGrid state and edit validation
013053b feat(runtime): add cross-provider page fallback
703cbc1 feat(ui): add QML-friendly navigation guard controller
7bc28f7 feat(widgetbridge): add QWidget QML focus bridge
```

## 3. 强制规则

### Git

- 直接在 main 开发。
- 不创建普通 feature branch。
- 每批使用语义清晰小提交。
- 临时 WIP 不要留在 main 历史；如需整理必须使用 lease-safe ref 更新。

### 架构

```text
LfQuickCore      -> QtCore only
LfQuickRuntime   -> Core，不允许 QWidget/QQuickItem
LfQuickUi        -> Core + Runtime + Qt Quick
LfQuickDataView  -> Ui + Qt Model/View/Quick
LfQuickPlatform  -> platform/input abstraction
WidgetBridge     -> optional Qt Widgets / QuickWidgets
Integration.*    -> legacy/product-specific adapters
```

不要把 EventFunc / LfWidget.dll / WindowManager / QXlsx / LimeReport / HookProc / ScanerHook / Patient-Sample-Reagent domain / medical Permission IDs / QC-Calibration 算法写进 Core/DataGrid。

### 完成标准

不要把空类、只有 QML 外壳、只有 API 声明记为完成。至少要求：

```text
contract/API
+ actual implementation
+ failure/edge handling
+ unit/integration tests where practical
+ UI capability has demo/vertical integration
+ architecture boundary review
```

CI runner 没实际执行时只能写“源码链路完成/待实编”，不能写 CI Green。

## 4. 当前关键能力

### Runtime / Migration

已具备：

- PageRouter / Lifecycle / History
- None / KeepAlive / Lru
- INavigationGuard
- Router-level cross-provider fallback
- QML create failure -> another provider -> WidgetPageProvider -> legacy QWidget
- provider diagnostics
- final provider-native ErrorPage

Runtime 不感知具体 UI 技术栈。

### Quick/UI

已具备：

- QuickRuntime / QuickPageProvider
- QmlErrorCollector / PageLoadError
- Overlay / Busy
- FocusNavigator
- NavigationGuardState / NavigationGuardController
- retry-once confirm workflow
- **PermissionEvaluator**

PermissionEvaluator 是 `IPermissionProvider` 的 QML adapter，支持：

```text
evaluate
allows
isVisible
isEnabled
isEditable
```

框架不保存业务 permission id。

### WidgetBridge

已具备：

- QuickSurface
- WidgetPageProvider
- WidgetFocusBridge / FocusTransferPolicy
- QWidget <-> QML Tab/Backtab
- focus remember/restore
- scanner isolation
- modal focus trap

### DataView

#### Columns / editing

已具备：

- stable column id
- ColumnStateModel
- ColumnConfigStore
- width/order/visibility persistence
- schemaVersion/tableId/modelSchema
- DataEditTransaction
- Temporary -> Validate -> Commit/Rollback

#### Typed delegates

已新增：

```text
DataGridEditorDelegate
DataGridCheckDelegate
DataGridCheckEditorDelegate
DataGridComboDelegate
DataGridComboEditorDelegate
DataGridProgressDelegate
DataGridIconDelegate
```

DataGrid resolver contract：

```text
cellDelegateResolver(columnId, logicalColumn)
editorDelegateResolver(columnId, logicalColumn)
editorOptionsResolver(columnId, logicalColumn)
```

Text/Check/Combo editor 配置 transaction 后都走 transaction，不能绕过 validator 直接修改 model。

#### Selection / context / keyboard

新增 `DataGridSelectionController` 和：

- selectedRowsList / selectedRowCount
- selectRow / selectRowsList / selectAllRows / clear
- current row/column
- batchActionRequested
- contextMenuRequested
- Home/End/Ctrl+Home/Ctrl+End
- PageUp/PageDown
- Left/Right skip hidden
- Tab/Backtab cell/row traversal
- nextGrid/previousGrid cross-table traversal

#### Proxy mapping

`DataFilterProxyModel`：

```text
mapRowToSource
mapRowFromSource
```

用于过滤/排序后批量动作。批量修改前应先 snapshot source rows，再改数据，避免 dynamic filter 改变 proxy row 排列。

#### Benchmark

Opt-in：

```cmake
-DLFQUICK_BUILD_BENCHMARKS=ON
```

目标：`lfquick_dataview_benchmark`

矩阵：20k/50k rows × 8/20/40 columns；更新 batch 1/10/100。输出 population/filter/sort/update latency CSV，不做硬件相关绝对阈值。

## 5. Medical Demo 当前实战

页面：

```text
Dashboard
Samples
Results
Results Review
Instrument
Alarms
Developer
```

### Results Review 已完成纵向链路

验证：

```text
DataGrid typed delegates
+ extended/batch selection
+ right-click context actions
+ proxy -> source row mapping
+ DataEditTransaction
+ Validation
+ PermissionEvaluator
+ NavigationGuard
+ Busy Overlay
+ column persistence
```

Synthetic roles：

- Reviewer -> Allow / editable
- Operator -> ReadOnly
- Guest -> Denied

Demo-specific id：

```text
demo.results.review
```

该 ID 只在 examples/product 侧，未进入 LfQuick framework。

Guard workflow：

```text
review note dirty
  -> route request blocked
  -> Application-level Dialog
  -> cancel stay / confirm retry once

saving review note
  -> busy hard block
  -> OverlayCoordinator busy token
  -> finish -> unblock
```

## 6. Tests

本批新增/扩展：

### tst_dataview

- typed DataGrid supporting infrastructure
- selection controller batch rows/current cell
- proxy visible row mapping
- filtered-out row -> -1
- invalid proxy row -> -1
- prior column persistence/edit transaction tests retained

### tst_ui

- PermissionEvaluator default Denied
- Allow
- ReadOnly
- Hidden
- Disabled
- unknown permission Denied
- prior Guard/Overlay/QmlError tests retained

### benchmark

- 20k/50k model-side benchmark source complete
- 尚未取得真实 Windows benchmark 数值，因为 Actions 未执行 Build。

## 7. CI 状态

最新功能 run：

```text
run: 38896094449
head: 9cdbdd2a156bd9231037d01bf3ea6e089873649d
job: windows-qt683
status: completed
conclusion: failure
steps: null
logs_url: null
```

仍为基础设施异常：没有 checkout / Install Qt / Configure / Build / Test。

必须继续使用：

```text
GitHub Actions runner/infrastructure abnormal
Real Qt 6.8.3 + MSVC2022 compile has NOT executed
Do not claim CI green
Do not claim source compile failure without compiler output
```

Runner 恢复后重点检查：

- DataGrid QML typed resolver/editDelegate/TapHandler
- Check/Combo editor commit
- DataGridSelectionController QML registrar
- Qt 6.8 TableView keyboard/position APIs
- PermissionEvaluator moc/QML registrar
- ResultsReview Menu/Timer/Dialogs
- QSortFilterProxyModel setData + row mapping
- benchmark MSVC compile
- prior fallback/guard/focus bridge

## 8. 下一开发批次

### Step 0 — Build Gate

每次先查 Actions。有实际 steps 就先修全部 Configure / Build / CTest。

### Step 1 — Sample Register（当前最高功能优先级）

必须做完整纵向页，不是 UI mock：

```text
Scanner
+ InputRouter
+ Validation
+ NavigationGuard
+ Overlay
+ form focus
```

建议：

- barcode/manual input conflict handling
- required field validation
- dirty guard
- submit busy state
- scanner exclusive session
- submit success clear/reset

医疗领域数据留在 Demo/Product，不进入 Core。

### Step 2 — Sample Rack

验证：

```text
DataGrid
+ scanner
+ focus/keyboard
+ batch selection/action
+ rack/position synthetic model
```

优先复用现有 DataGrid contract，不新增业务专用 DataGrid API。

### Step 3 — Permission attached API

现在已经有 `PermissionEvaluator`，下一步再封：

```qml
Lf.permission
```

但 attached API 只管理 UI experience；真正 Command/Domain 仍二次校验。

### Step 4 — Validation RuleTable

做通用 RuleTable / field feedback，避免每页手写一套重复 validation plumbing。

### Step 5 — UI consistency / stress

- Toast/Dialog Widget/QML dual stack
- DataGrid QML FPS/scroll/first-population benchmark
- Focus Bridge route-back/GUI stress
- QuickSurface DPI/resize/z-order stress

### Step 6 — 之后

- WindowRouter
- NavigationView（继续不要先做巨大版本）
- Charts abstraction
- Diagnostics HUD
- Hot Reload
- Qt 6.12 adapter

## 9. 已知债务

- Singleton 不是 route-global singleton。
- LRU 无 memory weight。
- NavigationGuardController 单 pending request。
- Focus Bridge boundary MVP，缺真实 GUI stress。
- ColumnConfigStore storage backend 尚未抽 interface。
- Permission attached property 未完成，但 evaluator 已完成。
- Validation RuleTable 未完成。
- QML render benchmark 未完成。
- Sample Register / Sample Rack 未完成。
- CI 尚未给出真实编译结果。

## 10. External Reference

继续遵守 `docs/REFERENCE_POLICY.md`：

- FluentUI MIT 公开源码允许研究；实际复制/实质衍生必须保留 MIT 记录；
- FluentUI2/Pro 只允许 public API/qmltypes/demo/behavior clean-room；
- 不逆向 DLL；
- 不还原私有 Pro 实现；
- LfQuick 自身架构优先于模仿 FluentUI。
