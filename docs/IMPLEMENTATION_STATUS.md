# LfQuick 当前实现状态

> 更新时间：2026-10-16  
> 开发策略：**main-only**。不为普通功能创建新分支；已有旧分支只做审查，不把被 main supersede 的早期实现重新合入。

## 当前基线

- Qt：6.8.3
- C++：20
- CMake：3.24+
- 目标升级：Qt 6.12 LTS
- 当前主线：`main`
- 本批功能 HEAD：`9cdbdd2a156bd9231037d01bf3ea6e089873649d`

## 本批新增提交

```text
67508a8 feat(dataview): add typed delegates and selection navigation
8bc8926 perf(dataview): add opt-in 20k 50k benchmark matrix
dd83729 feat(ui): add QML permission evaluator
2420b4f feat(dataview): expose proxy row mapping
9cdbdd2 feat(demo): add complete results review workflow
```

上一批关键提交：

```text
1b835b0 feat(dataview): persist stable DataGrid column state
19129fd feat(dataview): add validated edit transactions
7c54f74 feat(demo): exercise DataGrid state and edit validation
013053b feat(runtime): add cross-provider page fallback
703cbc1 feat(ui): add QML-friendly navigation guard controller
7bc28f7 feat(widgetbridge): add QWidget QML focus bridge
```

## Repository / Git

- main-only workflow 持续执行。
- 旧分支 `feat/runtime-foundation` 已确认属于被 main 覆盖的早期实现，不重新合入。
- 本批中间的临时 WIP 提交已通过 lease-safe ref rewrite 清理，main 保留语义化提交历史。
- 所有新增框架 API 保持产品/医疗领域无关。

## Core

已完成：

- Error / Result
- OperationHandle
- Permission / Validation contracts
- ICommand / CommandContext
- Event / EventDispatcher / RAII subscription
- RouteRequest / RouteResult

边界继续保持：`LfQuickCore -> QtCore only`。

## Runtime / Migration

已完成：

- PageDescriptor / PageRegistry / PageProviderRegistry
- IPageProvider / IPageInstance
- PageLifecycle
- PageRouter push / replace / back / reset
- NavigationGuard C++ contract
- None / KeepAlive / Lru cache
- Router 级 cross-provider fallback
- provider diagnostics
- QML failure -> another provider -> legacy QWidget
- final provider-native ErrorPage fallback

Runtime 继续只认 provider id，不感知 QWidget/QML/LfWidget/UnionLab。

## UI / Quick Runtime

已有：

- QuickRuntime / QuickPageProvider
- QmlErrorCollector / PageLoadError
- Overlay / Busy
- FocusNavigator
- NavigationGuardState / NavigationGuardController
- confirm/cancel/retry-once navigation workflow

### PermissionEvaluator — 本批新增

新增通用 QML adapter：

```text
IPermissionProvider
      ↓
PermissionEvaluator
      ↓
QML
```

支持 Core `PermissionState`：

```text
Allow
ReadOnly
Disabled
Hidden
Denied
```

QML API：

```text
evaluate(permissionId, context)
allows(...)
isVisible(...)
isEnabled(...)
isEditable(...)
```

规则：

- 框架不保存任何业务 permission id；
- 无 provider / 空 permission id 默认 Denied；
- ReadOnly 可显示/可交互查看，但不可编辑；
- 产品仍必须在 Command / Domain 层再次做权威权限检查，UI permission 只负责体验层。

`tst_ui` 已补 Allow / ReadOnly / Hidden / Disabled / Denied 映射测试。

## WidgetBridge

已有：

- QuickSurface
- WidgetPageProvider
- WidgetFocusBridge / FocusTransferPolicy
- QWidget <-> QML Tab/Backtab
- focus remember/restore
- scannerIsolation
- modal focus trap

仍需要真实 Qt 6.8.3 Windows GUI stress 验证。

## DataView

### 已完成基础

- DataTableModel
- DataFilterProxyModel
- DataGrid / TableView virtualization foundation
- stable column id
- ColumnStateModel
- ColumnConfigStore
- width/order/visibility persistence
- DataEditTransaction
- validated Commit/Rollback

### Typed delegates — 本批完成

新增 QML types：

```text
DataGridEditorDelegate
DataGridCheckDelegate
DataGridCheckEditorDelegate
DataGridComboDelegate
DataGridComboEditorDelegate
DataGridProgressDelegate
DataGridIconDelegate
```

DataGrid 新增 resolver contract：

```text
cellDelegateResolver(stableColumnId, logicalModelColumn)
editorDelegateResolver(stableColumnId, logicalModelColumn)
editorOptionsResolver(stableColumnId, logicalModelColumn)
```

关键约束：

- typed editor 不直接修改长期业务状态；
- 配置 `DataEditTransaction` 后，Text/Check/Combo editor 均走 Temporary -> Validate -> Commit/Rollback；
- validator 由页面/产品层提供；
- 列移动后继续使用 logical/model column，不使用 visual column 写 model；
- delegate 不包含 Sample / Patient / Reagent 等领域字段。

### Selection / Context / Keyboard — 本批完成

新增 `DataGridSelectionController`：

```text
selectedRowsList
selectedRowCount
selectRow
selectRowsList
selectAllRows
clear
setCurrentCell
moveCurrent
```

DataGrid 新增：

- batch row selection helper
- `batchActionRequested(actionId, selectedRows)`
- `contextMenuRequested(row, modelColumn, x, y, selectedRows)`
- right-click 自动保持/建立目标行选择
- Home / End
- Ctrl+Home / Ctrl+End
- PageUp / PageDown
- Left / Right 跳过隐藏列
- Tab / Backtab 跨单元格、跨行
- `nextGrid / previousGrid` 跨 DataGrid Tab/Backtab
- `focusFirstCell / focusLastCell / focusCell`

`tst_dataview` 已补 selection/current-cell/batch helper 测试。

### Proxy row mapping — 本批完成

`DataFilterProxyModel` 新增：

```text
mapRowToSource(proxyRow)
mapRowFromSource(sourceRow)
```

用于过滤/排序后的批量动作安全映射 source row。单测覆盖：

- visible row 双向映射；
- filtered-out source row -> -1；
- invalid proxy row -> -1。

### 20k / 50k benchmark — 本批完成源码

新增 opt-in 构建项：

```cmake
-DLFQUICK_BUILD_BENCHMARKS=ON
```

可执行程序：

```text
lfquick_dataview_benchmark
```

矩阵：

```text
Rows: 20k / 50k
Columns: 8 / 20 / 40
Update batch: 1 / 10 / 100
```

输出 CSV telemetry：

- population time
- filter time
- sort time
- update batch latency

不设置依赖 runner 硬件的绝对性能 PASS/FAIL 阈值。QML FPS / scroll latency 仍留给真实 GUI benchmark。

> Benchmark 源码/构建入口已完成，但由于 Actions runner 未执行 Build，目前还没有可信的 Qt 6.8.3 Windows benchmark 数值。

## Medical Demo

### 已有页面

- Dashboard
- Samples
- Results
- **Results Review**（本批新增）
- Instrument
- Alarms
- Developer

### Results Review — 本批完整纵向验证

`ResultsReviewPage.qml` 不只是视觉页，实际组合：

```text
DataGrid typed delegates
+ extended/batch selection
+ context-menu action contract
+ proxy row mapping
+ DataEditTransaction
+ Validation
+ PermissionEvaluator
+ NavigationGuard
+ Busy Overlay
+ column persistence
```

具体场景：

1. Reviewer / Operator / Guest 三种 synthetic role context；
2. Demo-specific permission id `demo.results.review` 只存在 examples/product 层；
3. Reviewer 可编辑/批量审核；Operator ReadOnly；Guest Denied；
4. Result 使用 Progress cell + Text editor；
5. Flag / Status 使用 Combo cell + Combo editor；
6. Result 数值、Flag、Status 均走 validator；
7. 多选后 Mark Reviewed / Hold；
8. 过滤后先 snapshot source rows，再变更状态，避免 proxy 动态重排导致误写；
9. 右键菜单使用当前 selectedRows；
10. review note 未保存 -> NavigationGuard confirm/cancel；
11. save note 期间 -> busy hard block；
12. save note 使用 OverlayCoordinator Busy token；
13. Medical Demo `App.qml` 增加全局 NavigationGuard confirm/hard-block Dialog 编排。

Demo 数据仍全部 synthetic，不是医疗诊断软件。

## CI 当前状态

本批结束前最新功能 run：

```text
run: 38896094449
head: 9cdbdd2a156bd9231037d01bf3ea6e089873649d
job: windows-qt683
status: completed
conclusion: failure
steps: null
logs_url: null
```

仍属于 GitHub Actions runner / infrastructure abnormal：job 创建后约 3 秒结束，没有 checkout / Install Qt / Configure / Build / Test。

因此当前必须保持：

```text
Architecture/source implementation: progressed
Functional vertical slices: implemented with unit tests/demo wiring
Real Qt 6.8.3 + MSVC2022 compile: NOT EXECUTED
CI Green: NO CLAIM
Source compile failure: NO COMPILER EVIDENCE
```

Runner 一旦恢复并出现真实 steps，第一优先级修完 Configure / Build / CTest，再继续扩展。

重点实编检查：

- DataGrid QML resolver / editDelegate / TapHandler registration
- Check/Combo editor commit behavior
- DataGridSelectionController QML_ELEMENT + QItemSelectionModel pointer property
- TableView positionViewAtCell / moveColumn API on Qt 6.8.3
- PermissionEvaluator moc/QML registration
- ResultsReviewPage Menu / Dialog / Timer / context properties
- QSortFilterProxyModel edit forwarding and proxy row mapping
- benchmark MSVC warnings/errors
- prior fallback / guard / WidgetFocusBridge GUI paths

## “完整实现”口径

本项目后续不把“有类/有 QML 文件”记为完成。一个能力只有至少满足以下条件才进入“已完成”清单：

```text
contract/API
+ implementation path
+ failure/edge handling
+ unit/integration test where practical
+ demo or real vertical usage for UI capability
+ architecture boundary review
```

当前 runner 不工作，因此“完成”表示源码功能链路完整并已做静态复核/测试代码覆盖，**不等于已经获得 Windows 实编通过证据**。

## 当前设计债务 / 未完成项

- PageCachePolicy::Singleton 仍非 route-global singleton。
- LRU 尚无 memory cost/weight。
- NavigationGuardController 当前是单 pending request。
- WidgetFocusBridge 仍需真实 GUI stress。
- ColumnConfigStore backend 目前内置 QSettings/INI，后续建议抽 `IColumnConfigBackend`。
- QML render/FPS/scroll benchmark 尚未建立稳定 GUI runner。
- Permission 当前已有 evaluator，尚未实现计划中的 `Lf.permission` attached property。
- Validation 仍缺 RuleTable / form feedback 通用层。
- Toast/Dialog Widget/QML dual-stack backend 尚未完成。
- Sample Register / Sample Rack 尚未实战化。

## 下一开发顺序

1. **Build gate**：Actions 一旦出现实际 steps，先修 Qt 6.8.3/MSVC2022 全量编译/CTest。
2. Medical Demo `Sample Register`：Scanner + Validation + NavigationGuard + Overlay。
3. Medical Demo `Sample Rack`：DataGrid + keyboard/focus + scanner + batch action。
4. Permission attached API `Lf.permission`，建立 evaluator/provider 的统一 UI context。
5. Validation RuleTable + 通用 form feedback。
6. Toast/Dialog dual-stack consistency。
7. DataGrid GUI benchmark：FPS / scroll / first population / memory telemetry。
8. Focus Bridge GUI stress / route-back restore hardening。
9. 再考虑 WindowRouter / NavigationView / Charts / Diagnostics / Hot Reload。

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

外部参考继续遵守 `docs/REFERENCE_POLICY.md`：FluentUI MIT 可在许可证下研究；FluentUI2/Pro 仅 public API/behavior/qmltypes/demo clean-room 参考，不逆向 DLL、不还原私有实现。
